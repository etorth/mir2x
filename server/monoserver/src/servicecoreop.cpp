/*
 * =====================================================================================
 *
 *       Filename: servicecoreop.cpp
 *        Created: 05/03/2016 21:29:58
 *  Last Modified: 05/28/2016 01:08:28
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
#include <string>

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

    OperateNet(stAMNP.SessionID, stAMNP.Type, stAMNP.Data, stAMNP.DataLen);
}

// this can only come from MonoServer layer, monster should be add internally
// void ServiceCore::On_MPK_ADDMONSTER(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
// {
//     AMAddMonster stAMAM;
//     std::memcpy(&stAMAM, rstMPK.Data(), sizeof(stAMAM));
//
//     // hummm, different map may have different size of RM
//     auto stRMAddr = GetRMAddress(stAMAM.MapID, stAMAM.X, stAMAM.Y);
//     if(stRMAddr == Theron::Address::Null()){
//         if(rstMPK.ID()){
//             m_ActorPod->Forward(MPK_ERROR, rstAddr, rstMPK.ID());
//         }
//     }
//
//
//
//     if(m_MapRecordMap.find(stAMAM.MapID) == m_MapRecordMap.end()){
//         LoadMap(stAMAM.MapID);
//     }
//
//     auto fnOPR = [this, rstFromAddr](const MessagePack &rstRMPK, const Theron::Address &){
//         switch(rstRMPK.Type()){
//             case MPK_OK:
//                 {
//                     m_ActorPod->Forward(MPK_OK, rstFromAddr);
//                     break;
//                 }
//             default:
//                 {
//                     m_ActorPod->Forward(MPK_ERROR, rstFromAddr);
//                     break;
//                 }
//         }
//     };
//     m_ActorPod->Forward({MPK_ADDMONSTER, rstMPK.Data(),
//             rstMPK.DataLen()}, m_MapRecordMap[stAMAM.MapID].PodAddress, fnOPR);
// }

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

// monoserver ask for adding a new monster, need response,  make it as simple as
// possible, since if the user found adding failed, they will add again, so
// there is no need to make a pending list of adding,if failed we just report
// the error to monoserver layer
void ServiceCore::On_MPK_ADDCHAROBJECT(const MessagePack &rstMPK, const Theron::Address &rstAddr)
{
    AMAddCharObject stAMACO;
    std::memcpy(&stAMACO, rstMPK.Data(), sizeof(stAMACO));

    uint32_t nMapID = stAMACO.Common.MapID;
    int nMapX = stAMACO.Common.MapX;
    int nMapY = stAMACO.Common.MapY;

    if(!ValidP(nMapID, nMapX, nMapY)){
        if(rstMPK.ID()){
            m_ActorPod->Forward(MPK_ERROR, rstAddr, rstMPK.ID());
        }
        return;
    }

    const auto &rstRMRecord = GetRMRecord(nMapID, nMapX, nMapY);
    switch(rstRMRecord.Query){
        case QUERY_NA:
            {
                // this RM is not walkable
                // TODO: this is a serious error actually
                //       we need take action rather than just report failure
                if(rstMPK.ID()){
                    m_ActorPod->Forward(MPK_ERROR, rstAddr, rstMPK.ID());
                }
                return;
            }
        case QUERY_OK:
            {
                // valid RM, try to login
                //
                // TODO just make sure...
                // if this happend, this is a serious error and actioin should be taken
                if(rstRMRecord.PodAddress == Theron::Address::Null()){
                    if(rstMPK.ID()){
                        m_ActorPod->Forward(MPK_ERROR, rstAddr, rstMPK.ID());
                    }
                    return;
                }

                // no respond, just forward, we don't need the response
                if(!rstMPK.ID()){
                    m_ActorPod->Forward({MPK_ADDCHAROBJECT, stAMACO}, rstRMRecord.PodAddress);
                    return;
                }

                // need respond
                auto fnDone = [nMPKID = rstMPK.ID(), rstAddr, this](
                        const MessagePack &rstRMPK, const Theron::Address &){
                    if(rstRMPK.Type() == MPK_OK){
                        m_ActorPod->Forward(MPK_OK, rstAddr, nMPKID);
                    }else{
                        m_ActorPod->Forward(MPK_ERROR, rstAddr, nMPKID);
                    }
                };

                m_ActorPod->Forward({MPK_ADDCHAROBJECT, stAMACO}, rstRMRecord.PodAddress, fnDone);
                return;
            }
        case QUERY_PENDING:
            {
                // the RM address is on the way
                // we need to adding the player when SC get this RM address
                std::string szRandomName;
                while(true){
                    szRandomName.clear();
                    for(int nCount = 0; nCount < 20; ++nCount){
                        szRandomName.push_back('A' + (std::rand() % ('Z' - 'A' + 1)));
                    }
                    if(!Installed(szRandomName)){ break; }
                }

                auto fnOnGetRMAddress = [stAMACO, nMapID, nMapX, nMapY, szRandomName, this](){
                    const auto &rstRMRecord = GetRMRecord(nMapID, nMapX, nMapY);

                    // still waiting
                    if(rstRMRecord.Query == QUERY_PENDING){ return; }

                    // ok we get valid address
                    if(rstRMRecord.Valid()){
                        m_ActorPod->Forward({MPK_ADDCHAROBJECT, stAMACO}, rstRMRecord.PodAddress);
                    }

                    m_ActorPod->Forward({MPK_ADDCHAROBJECT, stAMACO}, rstRMRecord.PodAddress);
                    Uninstall(szRandomName, true);
                };

                Install(szRandomName, fnOnGetRMAddress);
                return;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid query state");
                g_MonoServer->Restart();
                return;
            }
    }
}

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

    uint32_t nMapID = stAMLQDB.MapID;
    int nMapX = stAMLQDB.MapX;
    int nMapY = stAMLQDB.MapY;

    if(!ValidP(nMapID, nMapX, nMapY)){ return; }

    const auto &rstRMRecord = GetRMRecord(nMapID, nMapX, nMapY);
    switch(rstRMRecord.Query){
        case QUERY_NA:
            {
                // this RM is not walkable
                // TODO: this is a serious error actually
                //       we need take action rather than just report failure
                extern NetPodN *g_NetPodN;
                g_NetPodN->Send(stAMLQDB.SessionID, SM_LOGINFAIL,
                        [nSID = stAMLQDB.SessionID](){g_NetPodN->Shutdown(nSID);});
                return;
            }
        case QUERY_OK:
            {
                // valid RM, try to login
                if(rstRMRecord.PodAddress == Theron::Address::Null()){ return; }

                AMAddCharObject stAMACO;
                stAMACO.Type = OBJECT_PLAYER;
                stAMACO.Common.MapID     = stAMLQDB.MapID;
                stAMACO.Common.MapX      = stAMLQDB.MapX;
                stAMACO.Common.MapY      = stAMLQDB.MapY;
                stAMACO.Player.GUID      = stAMLQDB.GUID;
                stAMACO.Player.Level     = stAMLQDB.Level;
                stAMACO.Player.JobID     = stAMLQDB.JobID;
                stAMACO.Player.Direction = stAMLQDB.Direction;

                // we don't need the response
                // if adding succeeds, the player will report itself to SC
                m_ActorPod->Forward({MPK_ADDCHAROBJECT, stAMACO}, rstRMRecord.PodAddress);
                return;
            }
        case QUERY_PENDING:
            {
                // the RM address is on the way
                // we need to adding the player when SC get this RM address
                std::string szRandomName;
                while(true){
                    szRandomName.clear();
                    for(int nCount = 0; nCount < 20; ++nCount){
                        szRandomName.push_back('A' + (std::rand() % ('Z' - 'A' + 1)));
                    }
                    if(!Installed(szRandomName)){ break; }
                }

                auto fnOnGetRMAddress = [stAMLQDB, nMapID, nMapX, nMapY, szRandomName, this](){
                    const auto &rstRMRecord = GetRMRecord(nMapID, nMapX, nMapY);

                    // still pending
                    if(rstRMRecord.Query == QUERY_PENDING){ return; }

                    // OK we get the valid address
                    if(rstRMRecord.Valid()){
                        AMAddCharObject stAMACO;
                        stAMACO.Type = OBJECT_PLAYER;
                        stAMACO.Common.MapID     = stAMLQDB.MapID;
                        stAMACO.Common.MapX      = stAMLQDB.MapX;
                        stAMACO.Common.MapY      = stAMLQDB.MapY;
                        stAMACO.Player.GUID      = stAMLQDB.GUID;
                        stAMACO.Player.Level     = stAMLQDB.Level;
                        stAMACO.Player.JobID     = stAMLQDB.JobID;
                        stAMACO.Player.Direction = stAMLQDB.Direction;

                        // when adding succeed, the new object will respond
                        m_ActorPod->Forward({MPK_ADDCHAROBJECT, stAMACO}, rstRMRecord.PodAddress);
                    }

                    // TODO & TBD
                    // otherwise we just drop this operation

                    Uninstall(szRandomName, true);
                };

                Install(szRandomName, fnOnGetRMAddress);
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid query state");
                g_MonoServer->Restart();
            }
    }
}
