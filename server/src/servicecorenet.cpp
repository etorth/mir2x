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

    for(const auto [existChannID, existDBID]: m_dbidList){
        if(existChannID == channID){
            throw fflerror("internal error: channID reused before recycle: %d", to_d(channID));
        }

        if(existDBID == dbid){
            fnLoginError(LOGINERR_MULTILOGIN);
            return;
        }
    }

    m_dbidList[channID] = dbid;
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

    auto queryChar = g_dbPod->createQuery("select * from tbl_dbid where fld_dbid = %llu", to_llu(dbidOpt.value()));
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

    auto queryChar = g_dbPod->createQuery("select * from tbl_dbid where fld_dbid = %llu", to_llu(dbidOpt.value()));
    if(!queryChar.executeStep()){
        fnOnlineError(ONLINEERR_NOCHAR);
        return;
    }

    const int gender = queryChar.getColumn("fld_gender");
    const auto jobList = jobf::getJobList(queryChar.getColumn("fld_job"));
    const auto expectedUID = uidf::getPlayerUID(dbidOpt.value(), gender, jobList);

    fflassert(!g_actorPool->checkUIDValid(expectedUID));

    const int mapID = queryChar.getColumn("fld_map");
    const int mapX = queryChar.getColumn("fld_mapx");
    const int mapY = queryChar.getColumn("fld_mapy");

    const auto mapPtr = retrieveMap(mapID);
    fflassert(mapPtr);
    fflassert(mapPtr->In(mapID, mapX, mapY));

    AMAddCharObject amACO;
    std::memset(&amACO, 0, sizeof(amACO));

    amACO.type = UID_PLY;
    amACO.buf.assign(cerealf::serialize(SDInitPlayer
    {
        .dbid      = dbidOpt.value(),
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

    m_dbidList[channID] = dbidOpt.value();
    m_actorPod->forward(mapPtr->UID(), {AM_ADDCO, amACO}, [this, expectedUID, fnOnlineError](const ActorMsgPack &rmpk)
    {
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

    auto queryPassword = g_dbPod->createQuery(u8R"###( select * from tbl_account where fld_dbid = %llu and fld_password = '%s' )###", to_llu(dbidOpt.value()), to_cstr(cmDC.password));
    if(!queryPassword.executeStep()){
        fnDeleteCharError(DELCHARERR_BADPASSWORD);
        return;
    }

    auto query = g_dbPod->createQuery(u8R"###( delete from tbl_dbid where fld_dbid = %llu returning fld_dbid )###", to_llu(dbidOpt.value()));
    if(!query.executeStep()){
        fnDeleteCharError(DELCHARERR_NOCHAR);
        return;
    }

    g_dbPod->exec(u8R"###( delete from tbl_learnedmagiclist where fld_dbid = %llu returning fld_dbid )###", to_llu(dbidOpt.value()));

    const auto deletedDBID = check_cast<uint32_t, unsigned>(query.getColumn("fld_dbid"));
    fflassert(deletedDBID == dbidOpt.value());
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

    auto query = g_dbPod->createQuery(u8R"###(select fld_dbid, fld_name from tbl_dbid where fld_dbid = %llu or fld_name = '%s')###", to_llu(dbidOpt.value()), name.c_str());
    if(query.executeStep()){
        if(const auto existDBID = check_cast<uint32_t, unsigned>(query.getColumn("fld_dbid")); existDBID == dbidOpt.value()){
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
            u8R"###( insert into tbl_dbid(fld_dbid, fld_name, fld_job, fld_map, fld_mapx, fld_mapy, fld_gender) )###"
            u8R"###( values                                                                                     )###"
            u8R"###(     (%llu, '%s', '%s', '%d', %d, %d, %d);                                                  )###",

            to_llu(dbidOpt.value()),
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
