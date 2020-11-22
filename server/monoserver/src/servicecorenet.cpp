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
#include "dbpod.hpp"
#include "dbcomid.hpp"
#include "servermap.hpp"
#include "monoserver.hpp"
#include "dispatcher.hpp"
#include "servicecore.hpp"

extern DBPodN *g_DBPodN;
extern NetDriver *g_netDriver;
extern MonoServer *g_monoServer;

void ServiceCore::net_CM_Login(uint32_t nChannID, uint8_t, const uint8_t *pData, size_t)
{
    CMLogin stCML;
    std::memcpy(&stCML, pData, sizeof(stCML));

    auto fnOnLoginFail = [nChannID, stCML]()
    {
        g_monoServer->addLog(LOGTYPE_INFO, "Login failed for (%s:%s)", stCML.ID, "******");

        g_netDriver->Post(nChannID, SM_LOGINFAIL);
        g_netDriver->Shutdown(nChannID, false);
    };

    g_monoServer->addLog(LOGTYPE_INFO, "Login requested: (%s:%s)", stCML.ID, "******");
    auto pDBHDR = g_DBPodN->CreateDBHDR();

    if(!pDBHDR->QueryResult("select fld_id from tbl_account where fld_account = '%s' and fld_password = '%s'", stCML.ID, stCML.Password)){
        g_monoServer->addLog(LOGTYPE_INFO, "can't find account: (%s:%s)", stCML.ID, "******");

        fnOnLoginFail();
        return;
    }

    auto nID = pDBHDR->Get<int64_t>("fld_id");
    if(!pDBHDR->QueryResult("select * from tbl_dbid where fld_id = %d", (int)(nID))){
        g_monoServer->addLog(LOGTYPE_INFO, "no dbid created for this account: (%s:%s)", stCML.ID, "******");

        fnOnLoginFail();
        return;
    }

    AMLoginQueryDB stAMLQDBOK;
    std::memset(&stAMLQDBOK, 0, sizeof(stAMLQDBOK));

    auto nDBID      = pDBHDR->Get<int64_t>("fld_dbid");
    auto nMapID     = DBCOM_MAPID(pDBHDR->Get<std::u8string>("fld_mapname").c_str());
    auto nMapX      = pDBHDR->Get<int64_t>("fld_mapx");
    auto nMapY      = pDBHDR->Get<int64_t>("fld_mapy");
    auto nDirection = pDBHDR->Get<int64_t>("fld_direction");

    auto pMap = retrieveMap(nMapID);
    if(false
            || !pMap
            || !pMap->In(nMapID, nMapX, nMapY)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid db record found: (map, x, y) = (%d, %d, %d)", nMapID, nMapX, nMapY);

        fnOnLoginFail();
        return;
    }

    AMAddCharObject stAMACO;
    std::memset(&stAMACO, 0, sizeof(stAMACO));

    stAMACO.type             = UID_PLY;
    stAMACO.x                = nMapX;
    stAMACO.y                = nMapY;
    stAMACO.mapID            = nMapID;
    stAMACO.strictLoc        = false;
    stAMACO.player.DBID      = nDBID;
    stAMACO.player.direction = nDirection;
    stAMACO.player.channID   = nChannID;

    m_actorPod->forward(pMap->UID(), {MPK_ADDCHAROBJECT, stAMACO}, [this, fnOnLoginFail](const MessagePack &rstRMPK)
    {
        switch(rstRMPK.Type()){
            case MPK_OK:
                {
                    break;
                }
            default:
                {
                    fnOnLoginFail();
                    break;
                }
        }
    });
}
