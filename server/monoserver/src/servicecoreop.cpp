/*
 * =====================================================================================
 *
 *       Filename: servicecoreop.cpp
 *        Created: 05/03/2016 21:29:58
 *  Last Modified: 05/26/2016 00:18:03
 *
 *    Description: 
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

#include "player.hpp"
#include "actorpod.hpp"
#include "monoserver.hpp"
#include "servicecore.hpp"

// ServiceCore accepts net packages from *many* sessions and based on it to create
// the player object for a one to one map
//
// So servicecore <-> session is 1 to N, means we have to put put pointer of session
// in the net package otherwise we can't find the session even we have session's 
// address, session is a sync-driver, even we have it's address we can't find it
//
void ServiceCore::On_MPK_NETPACKAGE(const MessagePack &rstMPK, const Theron::Address &)
{
    AMNetPackage stAMNP;
    std::memcpy(&stAMNP, rstMPK.Data(), sizeof(stAMNP));

    OperateNet((Session *)(stAMNP.Session), stAMNP.Type, stAMNP.Data, stAMNP.DataLen);
}

void ServiceCore::On_MPK_ADDMONSTER(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMAddMonster stAMAM;
    std::memcpy(&stAMAM, rstMPK.Data(), sizeof(stAMAM));

    if(m_MapRecordMap.find(stAMAM.MapID) == m_MapRecordMap.end()){
        LoadMap(stAMAM.MapID);
    }

    auto fnOPR = [this, rstFromAddr](const MessagePack &rstRMPK, const Theron::Address &){
        switch(rstRMPK.Type()){
            case MPK_OK:
                {
                    m_ActorPod->Forward(MPK_OK, rstFromAddr);
                    break;
                }
            default:
                {
                    m_ActorPod->Forward(MPK_ERROR, rstFromAddr);
                    break;
                }
        }
    };
    m_ActorPod->Forward({MPK_ADDMONSTER, rstMPK.Data(),
            rstMPK.DataLen()}, m_MapRecordMap[stAMAM.MapID].PodAddress, fnOPR);
}

// TODO
// based on the information of coming connection
// do IP banning etc. currently only activate this session
void ServiceCore::On_MPK_NEWCONNECTION(const MessagePack &rstMPK, const Theron::Address &)
{
    uint32_t nSessionID = *((uint32_t *)rstMPK.Data());
    if(nSessionID){
        extern NetPodN *g_NetPodN;
        g_NetPodN->Launch(nSessionID, GetAddress());
    }
}

// void ServiceCore::On_MPK_LOGIN(const MessagePack &rstMPK, const Theron::Address &)
// {
//     AMLogin stAML;
//     std::memcpy(&stAML, rstMPK.Data(), sizeof(stAML));
//
//     extern MonoServer *g_MonoServer;
//     auto pNewPlayer = new Player(m_CurrUID++,
//             g_MonoServer->GetTimeTick(), stAML.GUID, stAML.SID);
//
//     // ... add all dress, inventory, weapon here
//     // ... add all position/direction/map/state here
//
//     AMNewPlayer stAMNP;
//     stAMNP.Data = (void *)pNewPlayer;
//
//     if(m_MapRecordMap.find(stAML.MapID) == m_MapRecordMap.end()){
//         // load map
//     }
//     m_ActorPod->Forward({MPK_NEWPLAYER, stAMNP}, m_MapRecordMap[stAML.MapID].PodAddress);
// }

void ServiceCore::On_MPK_PLAYERPHATOM(const MessagePack &rstMPK, const Theron::Address &)
{
    AMPlayerPhantom stAMPP;
    std::memcpy(&stAMPP, rstMPK.Data(), sizeof(stAMPP));

    if(m_PlayerRecordMap.find(stAMPP.GUID) != m_PlayerRecordMap.end()){
    }
}

// don't try to find its sender, it's from a temp SyncDriver in the lambda
void ServiceCore::On_MPK_LOGINQUERYDB(const MessagePack &rstMPK, const Theron::Address &)
{
    AMLoginQueryDB stAMLQDB;
    std::memcpy(&stAMLQDB, rstMPK.Data(), sizeof(stAMLQDB));

    auto pMap = m_MapRecordMap.find(stAMLQDB.MapID);

    // we didn't try to load it yet
    if(pMap == m_MapRecordMap.end()){
        LoadMap(stAMLQDB.MapID);
        pMap = m_MapRecordMap.find(stAMLQDB.MapID);
    }

    // mysterious error occurs...
    if(pMap == m_MapRecordMap.end()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "mysterious errors when loading map");
        g_MonoServer->Restart();
    }

    // tried, but turns out it's not valid
    if(pMap->second.PodAddress == Theron::Address::Null()){
        // this is not a valid map id
        // TODO: this is a serious error
        //       we need take action rather than just report failure
        extern NetPodN *g_NetPodN;
        g_NetPodN->Send(stAMLQDB.SessionID, SM_LOGINFAIL,
                [nSID = stAMLQDB.SessionID](){g_NetPodN->Shutdown(nSID);});
        return;
    }

    // ok, do something
    uint64_t nRMCacheKey = ((uint64_t)stAMLQDB.MapID << 32)
        + ((uint64_t)(stAMLQDB.MapX / SYS_MAPGRIDXP) << 16)
        + ((uint64_t)(stAMLQDB.MapY / SYS_MAPGRIDYP) <<  0);

    auto pRM = m_RMRecordMap.find(nRMCacheKey);

    if(pRM != m_RMRecordMap.end()){
        AMAddCharObject stAMACO;
        stAMACO.Type = OBJECT_PLAYER;
        stAMACO.Player.GUID      = stAMLQDB.GUID;
        stAMACO.Player.MapID     = stAMLQDB.MapID;
        stAMACO.Player.MapX      = stAMLQDB.MapX;
        stAMACO.Player.MapY      = stAMLQDB.MapY;
        stAMACO.Player.Level     = stAMLQDB.Level;
        stAMACO.Player.Job       = stAMLQDB.Job;
        stAMACO.Player.Direction = stAMLQDB.Direction;

        m_ActorPod->Forward({MPK_LOGIN, stAMACO}, pMap->second.PodAddress);
        return;
    }

    // when adding succeeds, the char object will response with this message
    auto fnOnCOInfo = [this](const MessagePack &rstRMPK, const Theron::Address &rstAddr){
        if(rstRMPK.Type() == MPK_CHAROBJECTINFO){
            AMCharObjectInfo stAMCOI;
            std::memcpy(&stAMCOI, rstRMPK.Data(), sizeof(stAMCOI));

            if(stAMCOI.Type == OBJECT_PLAYER){
                m_PlayerRecordMap[stAMCOI.Player.GUID] = {
                    stAMCOI.Player.GUID, stAMCOI.Player.UID, stAMCOI.Player.AddTime, rstAddr };
            }
        }
    };

    // ok we need to get the address first
    AMQueryRMAddress stAMQRMA;
    stAMQRMA.MapID = stAMLQDB.MapID;
    stAMQRMA.RMX  = stAMLQDB.MapX / SYS_MAPGRIDXP;
    stAMQRMA.RMY  = stAMLQDB.MapY / SYS_MAPGRIDYP;

    auto fnOnR = [this, stAMLQDB, nRMCacheKey, fnOnCOInfo](
            const MessagePack &rstRMPK, const Theron::Address &){
        switch(rstRMPK.Type()){
            case MPK_ADDRESS:
                {
                    Theron::Address stRMAddr = Theron::Address((char *)rstRMPK.Data());
                    m_RMRecordMap[nRMCacheKey] = {stAMLQDB.MapID,
                        (stAMLQDB.MapX / SYS_MAPGRIDXP), (stAMLQDB.MapY / SYS_MAPGRIDYP), stRMAddr};

                    AMAddCharObject stAMACO;
                    stAMACO.Type = OBJECT_PLAYER;
                    stAMACO.Player.GUID      = stAMLQDB.GUID;
                    stAMACO.Player.MapID     = stAMLQDB.MapID;
                    stAMACO.Player.MapX      = stAMLQDB.MapX;
                    stAMACO.Player.MapY      = stAMLQDB.MapY;
                    stAMACO.Player.Level     = stAMLQDB.Level;
                    stAMACO.Player.Job       = stAMLQDB.Job;
                    stAMACO.Player.Direction = stAMLQDB.Direction;

                    // when adding succeed, the new object will respond
                    m_ActorPod->Forward({MPK_ADDCHAROBJECT, stAMACO}, stRMAddr, fnOnCOInfo);
                    break;
                }
            default:
                {
                    m_RMRecordMap[nRMCacheKey] = {};
                    break;
                }
        }
    };
    m_ActorPod->Forward({MPK_QUERYRMADDRESS, stAMQRMA}, pMap->second.PodAddress, fnOnR);
}
