#include "jobf.hpp"
#include "mathf.hpp"
#include "dbpod.hpp"
#include "idstrf.hpp"
#include "dbcomid.hpp"
#include "netdriver.hpp"
#include "dbcomrecord.hpp"
#include "servermap.hpp"
#include "monoserver.hpp"
#include "dispatcher.hpp"
#include "protocoldef.hpp"
#include "servicecore.hpp"
#include "actorpool.hpp"

extern DBPod *g_dbPod;
extern NetDriver *g_netDriver;
extern MonoServer *g_monoServer;
extern ActorPool *g_actorPool;

void ServiceCore::net_CM_LOGIN(uint32_t channID, uint8_t, const uint8_t *buf, size_t)
{
    const auto cmL = ClientMsg::conv<CMLogin>(buf);
    const auto fnLoginError = [channID, &cmL](int error)
    {
        SMLoginError smLE;
        std::memset(&smLE, 0, sizeof(smLE));

        smLE.error = error;
        g_netDriver->post(channID, SM_LOGINERROR, &smLE, sizeof(smLE));
        g_monoServer->addLog(LOGTYPE_WARNING, "Login account failed: id = %s, ec = %d", to_cstr(cmL.id), error);
    };

    // don't check id/password by idstrf here
    // this allows some test account to be simple, like (test, 123456)

    auto queryAccount = g_dbPod->createQuery("select fld_dbid from tbl_account where fld_account = '%s' and fld_password = '%s'", to_cstr(cmL.id), to_cstr(cmL.password));
    if(!queryAccount.executeStep()){
        fnLoginError(LOGINERR_NOACCOUNT);
        return;
    }

    const auto dbid = check_cast<uint32_t, unsigned>(queryAccount.getColumn("fld_dbid"));
    fflassert(dbid);

    for(const auto &[existChannID, existDBID]: m_dbidList){
        if(existChannID == channID){
            throw fflerror("internal error: channID reused before recycle: %d", to_d(channID));
        }

        if(existDBID.first == dbid){
            fnLoginError(LOGINERR_MULTILOGIN);
            return;
        }
    }

    m_dbidList[channID] = std::make_pair(dbid, false);
    g_netDriver->post(channID, SM_LOGINOK, nullptr, 0);
}

void ServiceCore::net_CM_QUERYCHAR(uint32_t channID, uint8_t, const uint8_t *, size_t)
{
    const auto fnQueryCharError = [channID](int error)
    {
        SMQueryCharError smQCE;
        std::memset(&smQCE, 0, sizeof(smQCE));
        smQCE.error = error;
        g_netDriver->post(channID, SM_QUERYCHARERROR, &smQCE, sizeof(smQCE));
    };

    const auto dbidOpt = findDBID(channID);
    if(!dbidOpt.has_value()){
        fnQueryCharError(QUERYCHARERR_NOLOGIN);
        return;
    }

    auto queryChar = g_dbPod->createQuery("select * from tbl_char where fld_dbid = %llu", to_llu(dbidOpt.value().first));
    if(!queryChar.executeStep()){
        fnQueryCharError(QUERYCHARERR_NOCHAR);
        return;
    }

    SMQueryCharOK smQCOK;
    std::memset(&smQCOK, 0, sizeof(smQCOK));

    smQCOK.name.assign((std::string)(queryChar.getColumn("fld_name")));
    smQCOK.job.serialize(jobf::getJobList(queryChar.getColumn("fld_job")));
    smQCOK.gender = queryChar.getColumn("fld_gender");
    smQCOK.exp = queryChar.getColumn("fld_exp");
    g_netDriver->post(channID, SM_QUERYCHAROK, &smQCOK, sizeof(smQCOK));
}

void ServiceCore::net_CM_ONLINE(uint32_t channID, uint8_t, const uint8_t *, size_t)
{
    const auto fnOnlineError = [channID](int error)
    {
        SMOnlineError smOE;
        std::memset(&smOE, 0, sizeof(smOE));
        smOE.error = error;
        g_netDriver->post(channID, SM_ONLINEERROR, &smOE, sizeof(smOE));
    };

    const auto dbidOpt = findDBID(channID);
    if(!dbidOpt.has_value()){
        fnOnlineError(ONLINEERR_NOLOGIN);
        return;
    }

    if(dbidOpt.value().second){
        fnOnlineError(ONLINEERR_MULTIONLINE);
        return;
    }

    auto queryChar = g_dbPod->createQuery("select * from tbl_char where fld_dbid = %llu", to_llu(dbidOpt.value().first));
    if(!queryChar.executeStep()){
        fnOnlineError(ONLINEERR_NOCHAR);
        return;
    }

    const int gender = queryChar.getColumn("fld_gender");
    const auto jobList = jobf::getJobList(queryChar.getColumn("fld_job"));
    const auto expectedUID = uidf::getPlayerUID(dbidOpt.value().first, gender, jobList);

    fflassert(!g_actorPool->checkUIDValid(expectedUID));

    const int mapID = queryChar.getColumn("fld_map");
    const int mapX = queryChar.getColumn("fld_mapx");
    const int mapY = queryChar.getColumn("fld_mapy");

    const auto mapPtr = retrieveMap(mapID);
    fflassert(mapPtr);
    fflassert(mapPtr->in(mapID, mapX, mapY));

    AMAddCharObject amACO;
    std::memset(&amACO, 0, sizeof(amACO));

    amACO.type = UID_PLY;
    amACO.buf.assign(cerealf::serialize(SDInitPlayer
    {
        .dbid      = dbidOpt.value().first,
        .channID   = channID,
        .name      = queryChar.getColumn("fld_name"),
        .nameColor = queryChar.getColumn("fld_namecolor"),
        .x         = mapX,
        .y         = mapY,
        .mapID     = to_u32(mapID),
        .hp        = queryChar.getColumn("fld_hp"),
        .mp        = queryChar.getColumn("fld_mp"),
        .exp       = queryChar.getColumn("fld_exp"),
        .gold      = queryChar.getColumn("fld_gold"),
        .gender    = (bool)(gender),
        .jobList   = jobList,
        .hair      = queryChar.getColumn("fld_hair"),
        .hairColor = queryChar.getColumn("fld_haircolor"),
    }));

    m_dbidList[channID].second = true;
    m_actorPod->forward(mapPtr->UID(), {AM_ADDCO, amACO}, [channID, expectedUID, fnOnlineError, this](const ActorMsgPack &rmpk)
    {
        fflassert(findDBID(channID).has_value());
        fflassert(findDBID(channID).value().second);

        switch(rmpk.type()){
            case AM_UID:
                {
                    // don't need to send ONLINEOK here
                    // will be sent by Player actor with more parameters, not here

                    const auto amUID = rmpk.conv<AMUID>();
                    fflassert(amUID.UID == expectedUID);
                    return;
                }
            default:
                {
                    fnOnlineError(ONLINEERR_AMERROR);
                    m_dbidList[channID].second = false;
                    return;
                }
        }
    });
}

void ServiceCore::net_CM_CREATEACCOUNT(uint32_t channID, uint8_t, const uint8_t *buf, size_t)
{
    const auto fnCreateAccountError = [channID](int error)
    {
        SMCreateAccountError smCAE;
        std::memset(&smCAE, 0, sizeof(smCAE));
        smCAE.error = error;
        g_netDriver->post(channID, SM_CREATEACCOUNTERROR, &smCAE, sizeof(smCAE));
    };

    const auto cmCA = ClientMsg::conv<CMCreateAccount>(buf);
    const auto id = cmCA.id.to_str();
    const auto password = cmCA.password.to_str();

    if(!idstrf::isEmail(id.c_str())){
        fnCreateAccountError(CRTACCERR_BADACCOUNT);
        return;
    }

    if(!idstrf::isPassword(password.c_str())){
        fnCreateAccountError(CRTACCERR_BADPASSWORD);
        return;
    }

    if(g_dbPod->createQuery(u8R"###( select fld_dbid from tbl_account where fld_account ='%s' )###", id.c_str()).executeStep()){
        fnCreateAccountError(CRTACCERR_ACCOUNTEXIST);
        return;
    }

    g_dbPod->exec(u8R"###( insert into tbl_account(fld_account, fld_password) values ('%s', '%s') )###", id.c_str(), password.c_str());
    g_netDriver->post(channID, SM_CREATEACCOUNTOK, nullptr, 0);
}

void ServiceCore::net_CM_CHANGEPASSWORD(uint32_t channID, uint8_t, const uint8_t *buf, size_t)
{
    const auto fnChangePasswordError= [channID](int error)
    {
        SMChangePasswordError smCPE;
        std::memset(&smCPE, 0, sizeof(smCPE));
        smCPE.error = error;
        g_netDriver->post(channID, SM_CHANGEPASSWORDERROR, &smCPE, sizeof(smCPE));
    };

    const auto cmCP = ClientMsg::conv<CMChangePassword>(buf);
    const auto id = cmCP.id.to_str();
    const auto password = cmCP.password.to_str();
    const auto passwordNew = cmCP.passwordNew.to_str();

    if(!idstrf::isEmail(id.c_str())){
        fnChangePasswordError(CHGPWDERR_BADACCOUNT);
        return;
    }

    if(!idstrf::isPassword(password.c_str())){
        fnChangePasswordError(CHGPWDERR_BADPASSWORD);
        return;
    }

    if(!idstrf::isPassword(passwordNew.c_str())){
        fnChangePasswordError(CHGPWDERR_BADNEWPASSWORD);
        return;
    }

    auto query = g_dbPod->createQuery(
            u8R"###( update tbl_account set fld_password = '%s' where fld_account = '%s' and fld_password = '%s' returning * )###",

            passwordNew.c_str(),
            id.c_str(),
            password.c_str());

    if(!query.executeStep()){
        fnChangePasswordError(CHGPWDERR_BADACCOUNTPASSWORD);
        return;
    }
    g_netDriver->post(channID, SM_CHANGEPASSWORDOK, nullptr, 0);
}

void ServiceCore::net_CM_DELETECHAR(uint32_t channID, uint8_t, const uint8_t *buf, size_t)
{
    const auto fnDeleteCharError = [channID](int error)
    {
        SMDeleteCharError smDCE;
        std::memset(&smDCE, 0, sizeof(smDCE));
        smDCE.error = error;
        g_netDriver->post(channID, SM_DELETECHARERROR, &smDCE, sizeof(smDCE));
    };

    const auto cmDC = ClientMsg::conv<CMDeleteChar>(buf);
    const auto dbidOpt = findDBID(channID);

    if(!dbidOpt.has_value()){
        fnDeleteCharError(DELCHARERR_NOLOGIN);
        return;
    }

    auto queryPassword = g_dbPod->createQuery(u8R"###( select * from tbl_account where fld_dbid = %llu and fld_password = '%s' )###", to_llu(dbidOpt.value().first), to_cstr(cmDC.password));
    if(!queryPassword.executeStep()){
        fnDeleteCharError(DELCHARERR_BADPASSWORD);
        return;
    }

    auto query = g_dbPod->createQuery(u8R"###( delete from tbl_char where fld_dbid = %llu returning fld_dbid )###", to_llu(dbidOpt.value().first));
    if(!query.executeStep()){
        fnDeleteCharError(DELCHARERR_NOCHAR);
        return;
    }

    const auto dbidDeleted = check_cast<uint32_t, unsigned>(query.getColumn("fld_dbid"));
    fflassert(dbidDeleted == dbidOpt.value().first);

    g_dbPod->exec(u8R"###( delete from tbl_learnedmagiclist where fld_dbid = %llu returning fld_dbid )###", to_llu(dbidOpt.value().first));
    auto maxSeqID = [dbidOpt]() -> uint32_t
    {
        auto querySeqID = g_dbPod->createQuery(
                u8R"###( select                                                                           )###"
                u8R"###(     fld_seqid                                                                    )###"
                u8R"###( from                                                                             )###"
                u8R"###(     tbl_inventory                                                                )###"
                u8R"###( where                                                                            )###"
                u8R"###(     fld_dbid = %llu                                                              )###"
                u8R"###( and                                                                              )###"
                u8R"###(     fld_seqid = (select max(fld_seqid) from tbl_inventory where fld_dbid = %llu) )###",

                to_llu(dbidOpt.value().first),
                to_llu(dbidOpt.value().first));

        if(querySeqID.executeStep()){
            return (uint32_t)(querySeqID.getColumn("fld_seqid")) + 1;
        }
        else{
            return 1;
        }
    }();

    // delete tbl_belt and move them to tbl_inventory, create a long query string and execute query at one time
    // don't need to insert many times since tbl_belt items always has empty attributes

    size_t insertedBeltItemCount = 0;
    std::u8string insertQueryBeltString
    {
        u8R"###( insert into tbl_inventory(fld_dbid, fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist) )###"
        u8R"###( values                                                                                                                )###"
    };

    auto queryBelt = g_dbPod->createQuery(u8R"###( delete from tbl_belt where fld_dbid = %llu returning * )###", to_llu(dbidOpt.value().first));
    while(queryBelt.executeStep()){
        if(insertedBeltItemCount > 0){
            insertQueryBeltString += u8",";
        }

        const SDItem item
        {
            .itemID = check_cast<uint32_t, unsigned>(queryBelt.getColumn("fld_itemid")),
            .seqID  = maxSeqID++,
            .count  = check_cast<size_t, unsigned>(queryBelt.getColumn("fld_count")),

            // for rest of attributes take default values
            // don't assign default values for these attributes here manually
        };

        fflassert(item);
        insertedBeltItemCount++;
        insertQueryBeltString += str_printf(u8R"###( (%llu, %llu, %llu, %llu, %llu, %llu, ?) )###",
                to_llu(dbidOpt.value().first),
                to_llu(item.itemID),
                to_llu(item.seqID),
                to_llu(item.count),
                to_llu(item.duration[0]),
                to_llu(item.duration[1]));
    }

    const auto emptyAttrBuf = cerealf::serialize<SDItemExtAttrList>({});
    if(insertedBeltItemCount > 0){
        insertQueryBeltString += u8";";
        auto insertQuery = g_dbPod->createQuery(insertQueryBeltString.c_str());
        for(size_t i = 1; i <= insertedBeltItemCount; ++i){
            insertQuery.bind(i, emptyAttrBuf.data(), emptyAttrBuf.length());
        }
        insertQuery.exec();
    }

    // delete tbl_wear and move them to tbl_inventory
    // need to insert one by one since items in tbl_wear usually has non-empty extAttrList

    auto queryWear = g_dbPod->createQuery(u8R"###( delete from tbl_wear where fld_dbid = %llu returning * )###", to_llu(dbidOpt.value().first));
    while(queryWear.executeStep()){
        const SDItem item
        {
            .itemID = check_cast<uint32_t, unsigned>(queryWear.getColumn("fld_itemid")),
            .seqID  = maxSeqID++,
            .count  = check_cast<size_t, unsigned>(queryWear.getColumn("fld_count")),
            .duration
            {
                check_cast<size_t, unsigned>(queryWear.getColumn("fld_duration")),
                check_cast<size_t, unsigned>(queryWear.getColumn("fld_maxduration")),
            },

            // don't deserialize extAttrList
            // since it gets serialized quickly when inserted into tbl_inventory
        };

        fflassert(item);
        const std::string extAttrBuf = queryWear.getColumn("fld_extattrlist");

        auto insertQuery = g_dbPod->createQuery(
                u8R"###( insert into tbl_inventory(fld_dbid, fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist) )###"
                u8R"###( values                                                                                                                )###"
                u8R"###(     (%llu, %llu, %llu, %llu, %llu, %llu, ?)                                                                           )###",

                to_llu(dbidOpt.value().first),
                to_llu(item.itemID),
                to_llu(item.seqID),
                to_llu(item.count),
                to_llu(item.duration[0]),
                to_llu(item.duration[1]));

        insertQuery.bind(1, extAttrBuf.data(), extAttrBuf.length());
        insertQuery.exec();
    }

    // we don't move items in tbl_secureditemlist to tbl_inventory because tbl_secureditemlist has password
    // otherwise people can get secured items by just deleting char
    g_netDriver->post(channID, SM_DELETECHAROK, nullptr, 0);
}

void ServiceCore::net_CM_CREATECHAR(uint32_t channID, uint8_t, const uint8_t *buf, size_t)
{
    const auto cmCC = ClientMsg::conv<CMCreateChar>(buf);
    const auto fnCreateCharError = [channID](int error)
    {
        SMCreateCharError smCCE;
        std::memset(&smCCE, 0, sizeof(smCCE));
        smCCE.error = error;
        g_netDriver->post(channID, SM_CREATECHARERROR, &smCCE, sizeof(smCCE));
    };

    const auto dbidOpt = findDBID(channID);
    if(!dbidOpt.has_value()){
        fnCreateCharError(CRTCHARERR_NOLOGIN);
        return;
    }

    const auto name = cmCC.name.to_str();
    if(!idstrf::isCharName(name.c_str())){
        fnCreateCharError(CRTCHARERR_BADNAME);
        return;
    }

    auto query = g_dbPod->createQuery(u8R"###(select fld_dbid, fld_name from tbl_char where fld_dbid = %llu or fld_name = '%s')###", to_llu(dbidOpt.value().first), name.c_str());
    if(query.executeStep()){
        if(const auto existDBID = check_cast<uint32_t, unsigned>(query.getColumn("fld_dbid")); existDBID == dbidOpt.value().first){
            fnCreateCharError(CRTCHARERR_CHAREXIST);
            return;
        }
        else{
            fnCreateCharError(CRTCHARERR_NAMEEXIST);
            return;
        }
    }

    std::string dbError;
    try{
        g_dbPod->exec
        (
            u8R"###( insert into tbl_char(fld_dbid, fld_name, fld_job, fld_map, fld_mapx, fld_mapy, fld_gender) )###"
            u8R"###( values                                                                                     )###"
            u8R"###(     (%llu, '%s', '%s', '%d', %d, %d, %d);                                                  )###",

            to_llu(dbidOpt.value().first),
            to_cstr(cmCC.name),
            to_cstr(jobName(cmCC.job)),
            to_d(DBCOM_MAPID(u8"道馆_1")),
            405,
            120,
            to_d(cmCC.gender)
        );

        g_netDriver->post(channID, SM_CREATECHAROK, nullptr, 0);
        return;
    }
    catch(const std::exception &e){
        dbError = e.what();
    }
    catch(...){
        dbError = "unknown database error";
    }

    fnCreateCharError(CRTCHARERR_DBERROR);
    g_monoServer->addLog(LOGTYPE_WARNING, "Create char failed: %s", dbError.c_str());
}
