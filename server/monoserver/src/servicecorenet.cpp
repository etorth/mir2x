/*
 * =====================================================================================
 *
 *       Filename: servicecorenet.cpp
 *        Created: 05/20/2016 17:09:13
 *    Description: interaction btw NetPod and ServiceCore
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */
#include "jobf.hpp"
#include "dbpod.hpp"
#include "dbcomid.hpp"
#include "dbcomrecord.hpp"
#include "servermap.hpp"
#include "monoserver.hpp"
#include "dispatcher.hpp"
#include "protocoldef.hpp"
#include "servicecore.hpp"

extern DBPod *g_dbPod;
extern NetDriver *g_netDriver;
extern MonoServer *g_monoServer;

void ServiceCore::net_CM_Login(uint32_t channID, uint8_t, const uint8_t *buf, size_t)
{
    const auto cmL = ClientMsg::conv<CMLogin>(buf);
    const auto fnOnLoginFail = [channID, cmL](int error)
    {
        SMLoginFail smLF;
        std::memset(&smLF, 0, sizeof(smLF));

        smLF.error = error;
        g_netDriver->Post(channID, SM_LOGINFAIL, smLF);
        g_netDriver->Shutdown(channID, false);
        g_monoServer->addLog(LOGTYPE_WARNING, "Login failed for account: %s", cmL.id);
    };

    auto queryAccount = g_dbPod->createQuery("select fld_dbid from tbl_account where fld_account = '%s' and fld_password = '%s'", cmL.id, cmL.password);
    if(!queryAccount.executeStep()){
        fnOnLoginFail(LOGINERR_NOACCOUNT);
        return;
    }

    const auto dbid = check_cast<uint32_t, unsigned>(queryAccount.getColumn("fld_dbid"));
    auto queryChar = g_dbPod->createQuery("select * from tbl_dbid where fld_dbid = %llu", to_llu(dbid));
    if(!queryChar.executeStep()){
        fnOnLoginFail(LOGINERR_NODBID);
        return;
    }

    const int mapX = queryChar.getColumn("fld_mapx");
    const int mapY = queryChar.getColumn("fld_mapy");
    const uint32_t mapID = DBCOM_MAPID(to_u8cstr((const char *)(queryChar.getColumn("fld_mapname"))));

    const auto dbBuf = cerealf::serialize(SDInitPlayer
    {
        .dbid      = queryChar.getColumn("fld_dbid"),
        .channID   = channID,
        .name      = queryChar.getColumn("fld_name"),
        .nameColor = queryChar.getColumn("fld_namecolor"),
        .x         = mapX,
        .y         = mapY,
        .mapID     = mapID,
        .hp        = queryChar.getColumn("fld_hp"),
        .mp        = queryChar.getColumn("fld_mp"),
        .exp       = queryChar.getColumn("fld_exp"),
        .gold      = queryChar.getColumn("fld_gold"),
        .gender    = (bool)((int)(queryChar.getColumn("fld_gender"))),
        .jobList   = jobf::getJobList(queryChar.getColumn("fld_job")),
        .hair      = queryChar.getColumn("fld_hair"),
        .hairColor = queryChar.getColumn("fld_haircolor"),
    }, true);

    auto mapPtr = retrieveMap(mapID);
    if(!(mapPtr && mapPtr->In(mapID, mapX, mapY))){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid db record found: (map, x, y) = (%s, %d, %d)", to_cstr(DBCOM_MAPRECORD(mapID).name), mapX, mapY);
        fnOnLoginFail(LOGINERR_BADRECORD);
        return;
    }

    AMAddCharObject amACO;
    std::memset(&amACO, 0, sizeof(amACO));

    amACO.type = UID_PLY;
    if(dbBuf.length() > sizeof(amACO.buf.data)){
        throw fflerror("actor message buffer is too small");
    }

    amACO.buf.size = dbBuf.length();
    std::copy(dbBuf.begin(), dbBuf.end(), amACO.buf.data);

    m_actorPod->forward(mapPtr->UID(), {AM_ADDCHAROBJECT, amACO}, [this, fnOnLoginFail](const ActorMsgPack &rmpk)
    {
        switch(rmpk.type()){
            case AM_OK:
                {
                    break;
                }
            default:
                {
                    fnOnLoginFail(LOGINERR_UNKNOWN);
                    break;
                }
        }
    });
}
