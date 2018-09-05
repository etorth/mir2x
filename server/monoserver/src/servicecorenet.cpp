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

void ServiceCore::Net_CM_Login(uint32_t nChannID, uint8_t, const uint8_t *pData, size_t)
{
    CMLogin stCML;
    std::memcpy(&stCML, pData, sizeof(stCML));

    auto fnOnLoginFail = [nChannID, stCML]()
    {
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_INFO, "Login failed for (%s:%s)", stCML.ID, "******");

        extern NetDriver *g_NetDriver;
        g_NetDriver->Post(nChannID, SM_LOGINFAIL);
        g_NetDriver->Shutdown(nChannID, false);
    };

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_INFO, "Login requested: (%s:%s)", stCML.ID, "******");

    extern DBPodN *g_DBPodN;
    auto pDBHDR = g_DBPodN->CreateDBHDR();

    if(!pDBHDR->Execute("select fld_id from tbl_account where fld_account = '%s' and fld_password = '%s'", stCML.ID, stCML.Password)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "SQL ERROR: (%d: %s)", pDBHDR->ErrorID(), pDBHDR->ErrorInfo());

        fnOnLoginFail();
        return;
    }

    if(pDBHDR->RowCount() < 1){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_INFO, "can't find account: (%s:%s)", stCML.ID, "******");

        fnOnLoginFail();
        return;
    }

    pDBHDR->Fetch();

    auto nID = std::atoi(pDBHDR->Get("fld_id"));
    if(!pDBHDR->Execute("select * from mir2x.tbl_dbid where fld_id = %d", nID)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "SQL ERROR: (%d: %s)", pDBHDR->ErrorID(), pDBHDR->ErrorInfo());

        fnOnLoginFail();
        return;
    }

    if(pDBHDR->RowCount() < 1){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_INFO, "no dbid created for this account: (%s:%s)", stCML.ID, "******");

        fnOnLoginFail();
        return;
    }

    AMLoginQueryDB stAMLQDBOK;
    std::memset(&stAMLQDBOK, 0, sizeof(stAMLQDBOK));

    pDBHDR->Fetch();

    auto nDBID      = std::atoi(pDBHDR->Get("fld_dbid"));
    auto nMapID     = DBCOM_MAPID(pDBHDR->Get("fld_mapname"));
    auto nMapX      = std::atoi(pDBHDR->Get("fld_mapx"));
    auto nMapY      = std::atoi(pDBHDR->Get("fld_mapy"));
    auto nDirection = std::atoi(pDBHDR->Get("fld_direction"));

    auto pMap = RetrieveMap(nMapID);
    if(false
            || !pMap
            || !pMap->In(nMapID, nMapX, nMapY)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid db record found: (map, x, y) = (%d, %d, %d)", nMapID, nMapX, nMapY);

        fnOnLoginFail();
        return;
    }

    AMAddCharObject stAMACO;
    std::memset(&stAMACO, 0, sizeof(stAMACO));

    stAMACO.Type             = TYPE_PLAYER;
    stAMACO.Common.MapID     = nMapID;
    stAMACO.Common.X         = nMapX;
    stAMACO.Common.Y         = nMapY;
    stAMACO.Common.Random    = true;
    stAMACO.Player.DBID      = nDBID;
    stAMACO.Player.Direction = nDirection;
    stAMACO.Player.ChannID   = nChannID;

    m_ActorPod->Forward(pMap->UID(), {MPK_ADDCHAROBJECT, stAMACO}, [this, fnOnLoginFail](const MessagePack &rstRMPK)
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
