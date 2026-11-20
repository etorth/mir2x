#include "jobf.hpp"
#include "uidf.hpp"
#include "uidsf.hpp"
#include "mathf.hpp"
#include "dbpod.hpp"
#include "idstrf.hpp"
#include "dbcomid.hpp"
#include "servermap.hpp"
#include "server.hpp"
#include "dispatcher.hpp"
#include "protocoldef.hpp"
#include "servicecore.hpp"
#include "actorpool.hpp"

extern DBPod *g_dbPod;
extern Server *g_server;
extern ActorPool *g_actorPool;

corof::awaitable<> ServiceCore::net_CM_LOGIN(uint32_t channID, uint8_t, const uint8_t *buf, size_t, uint64_t respID)
{
    const auto cmL = ClientMsg::conv<CMLogin>(buf);
    const auto fnLoginError = [channID, &cmL, respID, this](int error)
    {
        SMLoginError smLE;
        std::memset(&smLE, 0, sizeof(smLE));

        smLE.error = error;
        m_actorPod->postNet(channID, SM_LOGINERROR, &smLE, sizeof(smLE), respID);
        g_server->addLog(LOGTYPE_WARNING, "Login account failed: id = %s, ec = %d", cmL.id.as_cstr(), error);
    };

    // don't check id/password by idstrf here
    // this allows some test account to be simple, like (test, 123456)

    auto queryAccount = g_dbPod->createQuery("select fld_account from tbl_account where fld_account = ? and fld_password = ?");
    queryAccount.bind(1, cmL.id.as_cstr());
    queryAccount.bind(2, cmL.password.as_cstr());

    if(!queryAccount.executeStep()){
        fnLoginError(LOGINERR_NOACCOUNT);
        return {};
    }

    const std::string account = queryAccount.getColumn("fld_account");
    fflassert(!account.empty());

    for(const auto &[existChannID, session]: m_sessionList){
        if(existChannID == channID){
            throw fflpanic("internal error: channID reused before recycle: {}", channID);
        }

        if(session.account == account){
            fnLoginError(LOGINERR_MULTILOGIN);
            return {};
        }
    }

    m_sessionList[channID] = LoginSession
    {
        .account = account,
    };
    m_actorPod->postNet(channID, SM_LOGINOK, nullptr, 0, respID);
    return {};
}

corof::awaitable<> ServiceCore::net_CM_QUERYCHAR(uint32_t channID, uint8_t, const uint8_t *, size_t, uint64_t respID)
{
    const auto fnQueryCharError = [channID, respID, this](int error)
    {
        SMQueryCharError smQCE;
        std::memset(&smQCE, 0, sizeof(smQCE));
        smQCE.error = error;
        m_actorPod->postNet(channID, SM_QUERYCHARERROR, &smQCE, sizeof(smQCE), respID);
    };

    const auto session = findSession(channID);
    if(!session.has_value()){
        fnQueryCharError(QUERYCHARERR_NOLOGIN);
        return {};
    }

    auto queryChar = g_dbPod->createQuery("select * from tbl_char where fld_account = ?");
    queryChar.bind(1, session->account);

    if(!queryChar.executeStep()){
        fnQueryCharError(QUERYCHARERR_NOCHAR);
        return {};
    }

    SMQueryCharOK smQCOK;
    std::memset(&smQCOK, 0, sizeof(smQCOK));

    smQCOK.name.assign((std::string)(queryChar.getColumn("fld_name")));
    smQCOK.gender = queryChar.getColumn("fld_gender");
    smQCOK.job = queryChar.getColumn("fld_job");
    smQCOK.exp = to_uz(queryChar.getColumn("fld_exp").getInt64());
    m_actorPod->postNet(channID, SM_QUERYCHAROK, &smQCOK, sizeof(smQCOK), respID);
    return {};
}

corof::awaitable<> ServiceCore::net_CM_ONLINE(uint32_t channID, uint8_t, const uint8_t *, size_t, uint64_t respID)
{
    const auto fnOnlineError = [channID, respID, this](int error)
    {
        SMOnlineError smOE;
        std::memset(&smOE, 0, sizeof(smOE));
        smOE.error = error;
        m_actorPod->postNet(channID, SM_ONLINEERROR, &smOE, sizeof(smOE), respID);
    };

    const auto session = findSession(channID);
    if(!session.has_value()){
        fnOnlineError(ONLINEERR_NOLOGIN);
        co_return;
    }

    if(session->dbid){
        fnOnlineError(ONLINEERR_MULTIONLINE);
        co_return;
    }

    const auto dbid = [&session]() -> uint32_t
    {
        auto queryDBID = g_dbPod->createQuery("select fld_dbid from tbl_char where fld_account = ?");
        queryDBID.bind(1, session->account);

        if(queryDBID.executeStep()){
            return check_cast<uint32_t, unsigned>(queryDBID.getColumn("fld_dbid"));
        }
        return 0;
    }();

    if(!dbid){
        fnOnlineError(ONLINEERR_NOCHAR);
        co_return;
    }

    const auto playerUID = uidf::getPlayerUID(dbid);
    if(g_actorPool->checkUIDExist(playerUID)){
        // player actor is hosted by 离线挂机, rebind it to this channel
        // mark online before co_await, another CM_ONLINE during the co_await gets ONLINEERR_MULTIONLINE
        m_sessionList[channID].dbid = dbid;

        AMRebindChannel amRC;
        std::memset(&amRC, 0, sizeof(amRC));

        amRC.channID = channID;
        if(const auto rmpk = co_await m_actorPod->send(playerUID, {AM_REBINDCHANNEL, amRC}); rmpk.type() == AM_OK){
            co_return;
        }

        // player is logging out, wait until it's gone, then load it from database again
        // channel can be closed during co_await, then AM_BADCHANNEL already erased the entry
        while(true){
            auto p = m_sessionList.find(channID);
            if((p == m_sessionList.end()) || (p->second.account != session->account)){
                co_return;
            }

            if(!g_actorPool->checkUIDExist(playerUID)){
                p->second.dbid = 0; // spawn a new actor as normal online
                break;
            }
            co_await asyncWait(10);
        }
    }

    auto queryChar = g_dbPod->createQuery("select * from tbl_char where fld_dbid = %llu", to_llu(dbid));
    if(!queryChar.executeStep()){
        fnOnlineError(ONLINEERR_NOCHAR);
        co_return;
    }

    const int mapID = queryChar.getColumn("fld_map");
    const int mapX  = queryChar.getColumn("fld_mapx");
    const int mapY  = queryChar.getColumn("fld_mapy");

    const uint64_t mapUID = uidsf::getBaseMapUID(mapID); // same mapID always maps to same UID
    SDInitCharObject sdICO = SDInitPlayer
    {
        .dbid      = dbid,
        .channID   = channID,
        .name      = queryChar.getColumn("fld_name"),
        .mapUID    = mapUID,
        .x         = mapX,
        .y         = mapY,
        .hp        = queryChar.getColumn("fld_hp"),
        .mp        = queryChar.getColumn("fld_mp"),
        .exp       = to_uz(queryChar.getColumn("fld_exp").getInt64()),
        .gold      = to_uz(queryChar.getColumn("fld_gold").getInt64()),
        .gender    = queryChar.getColumn("fld_gender").getUInt() > 0,
        .job       = queryChar.getColumn("fld_job"),
        .hair      = queryChar.getColumn("fld_hair"),
        .hairColor = queryChar.getColumn("fld_haircolor"),
        .pkPoint   = queryChar.getColumn("fld_pkpoint"),
        .viewRange = queryChar.getColumn("fld_viewr"),
    };

    AMLoadMap amLM;
    std::memset(&amLM, 0, sizeof(amLM));

    amLM.mapUID = mapUID;
    amLM.waitActivated = false;

    if(const auto [loaded, _] = co_await requestLoadMap(amLM); loaded){
        if(m_addCO->addCO(sdICO)){
            m_sessionList[channID].dbid = dbid;
        }
        else{
            m_sessionList[channID].dbid = 0;
            fnOnlineError(ONLINEERR_AMERROR);
        }
    }
    else{
        throw fflpanic("failed to load map: {}", uidf::getUIDString(mapUID));
    }
}

corof::awaitable<> ServiceCore::net_CM_CREATEACCOUNT(uint32_t channID, uint8_t, const uint8_t *buf, size_t, uint64_t respID)
{
    const auto fnCreateAccountError = [channID, respID, this](int error)
    {
        SMCreateAccountError smCAE;
        std::memset(&smCAE, 0, sizeof(smCAE));
        smCAE.error = error;
        m_actorPod->postNet(channID, SM_CREATEACCOUNTERROR, &smCAE, sizeof(smCAE), respID);
    };

    const auto cmCA = ClientMsg::conv<CMCreateAccount>(buf);
    const auto id = cmCA.id.to_str();
    const auto password = cmCA.password.to_str();

    if(!idstrf::isEmail(id.c_str())){
        fnCreateAccountError(CRTACCERR_BADACCOUNT);
        return {};
    }

    if(!idstrf::isPassword(password.c_str())){
        fnCreateAccountError(CRTACCERR_BADPASSWORD);
        return {};
    }

    auto queryAccount = g_dbPod->createQuery(u8R"###( select fld_account from tbl_account where fld_account = ? )###");
    queryAccount.bind(1, id);

    if(queryAccount.executeStep()){
        fnCreateAccountError(CRTACCERR_ACCOUNTEXIST);
        return {};
    }

    auto queryInsert = g_dbPod->createQuery(u8R"###( insert into tbl_account(fld_account, fld_password) values (?, ?) )###");
    queryInsert.bind(1, id);
    queryInsert.bind(2, password);
    queryInsert.exec();

    m_actorPod->postNet(channID, SM_CREATEACCOUNTOK, nullptr, 0, respID);
    return {};
}

corof::awaitable<> ServiceCore::net_CM_CHANGEPASSWORD(uint32_t channID, uint8_t, const uint8_t *buf, size_t, uint64_t respID)
{
    const auto fnChangePasswordError= [channID, respID, this](int error)
    {
        SMChangePasswordError smCPE;
        std::memset(&smCPE, 0, sizeof(smCPE));
        smCPE.error = error;
        m_actorPod->postNet(channID, SM_CHANGEPASSWORDERROR, &smCPE, sizeof(smCPE), respID);
    };

    const auto cmCP = ClientMsg::conv<CMChangePassword>(buf);
    const auto id = cmCP.id.to_str();
    const auto password = cmCP.password.to_str();
    const auto passwordNew = cmCP.passwordNew.to_str();

    if(!idstrf::isEmail(id.c_str())){
        fnChangePasswordError(CHGPWDERR_BADACCOUNT);
        return {};
    }

    if(!idstrf::isPassword(password.c_str())){
        fnChangePasswordError(CHGPWDERR_BADPASSWORD);
        return {};
    }

    if(!idstrf::isPassword(passwordNew.c_str())){
        fnChangePasswordError(CHGPWDERR_BADNEWPASSWORD);
        return {};
    }

    auto query = g_dbPod->createQuery(u8R"###( update tbl_account set fld_password = ? where fld_account = ? and fld_password = ? returning * )###");
    query.bind(1, passwordNew);
    query.bind(2, id);
    query.bind(3, password);

    if(!query.executeStep()){
        fnChangePasswordError(CHGPWDERR_BADACCOUNTPASSWORD);
        return {};
    }
    m_actorPod->postNet(channID, SM_CHANGEPASSWORDOK, nullptr, 0, respID);
    return {};
}

corof::awaitable<> ServiceCore::net_CM_DELETECHAR(uint32_t channID, uint8_t, const uint8_t *buf, size_t, uint64_t respID)
{
    const auto fnDeleteCharError = [channID, respID, this](int error)
    {
        SMDeleteCharError smDCE;
        std::memset(&smDCE, 0, sizeof(smDCE));
        smDCE.error = error;
        m_actorPod->postNet(channID, SM_DELETECHARERROR, &smDCE, sizeof(smDCE), respID);
    };

    const auto cmDC = ClientMsg::conv<CMDeleteChar>(buf);
    const auto session = findSession(channID);

    if(!session.has_value()){
        fnDeleteCharError(DELCHARERR_NOLOGIN);
        return {};
    }

    auto queryAccount = g_dbPod->createQuery(u8R"###( select fld_account from tbl_account where fld_account = ? and fld_password = ? )###");
    queryAccount.bind(1, session->account);
    queryAccount.bind(2, cmDC.password.as_cstr());

    if(!queryAccount.executeStep()){
        fnDeleteCharError(DELCHARERR_BADPASSWORD);
        return {};
    }

    const auto dbid = [&session]() -> uint32_t
    {
        auto queryDBID = g_dbPod->createQuery(u8R"###( select fld_dbid from tbl_char where fld_account = ? )###");
        queryDBID.bind(1, session->account);

        if(queryDBID.executeStep()){
            return check_cast<uint32_t, unsigned>(queryDBID.getColumn("fld_dbid"));
        }
        return 0;
    }();

    if(!dbid){
        fnDeleteCharError(DELCHARERR_NOCHAR);
        return {};
    }

    // a hosted (离线挂机) or logging-out actor still holds this char and keeps writing rows of it
    if(g_actorPool->checkUIDExist(uidf::getPlayerUID(dbid))){
        fnDeleteCharError(DELCHARERR_HOSTED);
        return {};
    }

    // all rows of this char go by foreign key cascade
    try{
        g_dbPod->exec(u8R"###( delete from tbl_char where fld_dbid = %llu )###", to_llu(dbid));
    }
    catch(const std::exception &e){
        g_server->addLog(LOGTYPE_WARNING, "Delete char failed: %s", e.what());
        fnDeleteCharError(DELCHARERR_DBERROR);
        return {};
    }
    catch(...){
        g_server->addLog(LOGTYPE_WARNING, "Delete char failed: unknown database error");
        fnDeleteCharError(DELCHARERR_DBERROR);
        return {};
    }

    m_actorPod->postNet(channID, SM_DELETECHAROK, nullptr, 0, respID);
    return {};
}

corof::awaitable<> ServiceCore::net_CM_CREATECHAR(uint32_t channID, uint8_t, const uint8_t *buf, size_t, uint64_t respID)
{
    const auto cmCC = ClientMsg::conv<CMCreateChar>(buf);
    const auto fnCreateCharError = [channID, respID, this](int error)
    {
        SMCreateCharError smCCE;
        std::memset(&smCCE, 0, sizeof(smCCE));
        smCCE.error = error;
        m_actorPod->postNet(channID, SM_CREATECHARERROR, &smCCE, sizeof(smCCE), respID);
    };

    const auto session = findSession(channID);
    if(!session.has_value()){
        fnCreateCharError(CRTCHARERR_NOLOGIN);
        return {};
    }

    const auto name = cmCC.name.to_str();
    if(!idstrf::isCharName(name.c_str())){
        fnCreateCharError(CRTCHARERR_BADNAME);
        return {};
    }

    if(cmCC.gender > 1){
        fnCreateCharError(CRTCHARERR_BADGENDER);
        return {};
    }

    if(!jobf::jobValid(cmCC.job)){
        fnCreateCharError(CRTCHARERR_BADJOB);
        return {};
    }

    auto queryExist = g_dbPod->createQuery(u8R"###(select fld_account from tbl_char where fld_account = ? or fld_name = ?)###");
    queryExist.bind(1, session->account);
    queryExist.bind(2, name);

    if(queryExist.executeStep()){
        if(const std::string existAccount = queryExist.getColumn("fld_account"); existAccount == session->account){
            fnCreateCharError(CRTCHARERR_CHAREXIST);
            return {};
        }
        else{
            fnCreateCharError(CRTCHARERR_NAMEEXIST);
            return {};
        }
    }

    std::string dbError;
    try{
        auto dbTrans = g_dbPod->createTransaction();
        const auto dbid = [&session, &cmCC, &name]() -> uint64_t
        {
            auto queryInsert = g_dbPod->createQuery
            (
                u8R"###( insert into tbl_char(fld_account, fld_name, fld_gender, fld_job, fld_map, fld_mapx, fld_mapy) )###"
                u8R"###( values                                                                                        )###"
                u8R"###(     (?, ?, %d, %d, %d, %d, %d)                                                                )###"
                u8R"###( returning fld_dbid                                                                            )###",

                to_d(cmCC.gender),
                to_d(cmCC.job),
                to_d(DBCOM_MAPID(u8"道馆_1")),
                405,
                120
            );

            queryInsert.bind(1, session->account);
            queryInsert.bind(2, name);

            if(!queryInsert.executeStep()){
                throw fflpanic("insert into tbl_char returns no dbid");
            }
            return check_cast<uint64_t>(queryInsert.getColumn("fld_dbid").getInt64());
        }();

        // dbid above SYS_MAXDBID is reserved for chat peers like system, group and AI
        // dbTrans rolls back the insert without commit

        if(dbid <= SYS_MAXDBID){
            dbTrans.commit();
            m_actorPod->postNet(channID, SM_CREATECHAROK, nullptr, 0, respID);
            return {};
        }
        dbError = str_printf("dbid exceeds SYS_MAXDBID: %llu", to_llu(dbid));
    }
    catch(const std::exception &e){
        dbError = e.what();
    }
    catch(...){
        dbError = "unknown database error";
    }

    fnCreateCharError(CRTCHARERR_DBERROR);
    g_server->addLog(LOGTYPE_WARNING, "Create char failed: %s", dbError.c_str());
    return {};
}
