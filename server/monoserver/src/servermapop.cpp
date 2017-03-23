/*
 * =====================================================================================
 *
 *       Filename: servermapop.cpp
 *        Created: 05/03/2016 20:21:32
 *  Last Modified: 03/23/2017 00:12:29
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
#include "monster.hpp"
#include "mathfunc.hpp"
#include "sysconst.hpp"
#include "actorpod.hpp"
#include "metronome.hpp"
#include "servermap.hpp"
#include "monoserver.hpp"

void ServerMap::On_MPK_HI(const MessagePack &, const Theron::Address &)
{
}

void ServerMap::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    for(auto &rstRecordLine: m_ObjectV2D){
        for(auto &rstRecordV: rstRecordLine){
            for(auto pObject: rstRecordV){
                if(pObject && pObject->Active()){
                    m_ActorPod->Forward(MPK_METRONOME, ((ActiveObject *)(pObject))->GetAddress());
                }
            }
        }
    }
}

void ServerMap::On_MPK_ACTIONSTATE(const MessagePack &rstMPK, const Theron::Address &)
{
    AMActionState stAMAS;
    std::memcpy(&stAMAS, rstMPK.Data(), sizeof(stAMAS));

    int nX0 = (stAMAS.X - SYS_MAPVISIBLECD);
    int nX1 = (stAMAS.X + SYS_MAPVISIBLECD);
    int nY0 = (stAMAS.Y - SYS_MAPVISIBLECD);
    int nY1 = (stAMAS.Y + SYS_MAPVISIBLECD);

    for(int nX = nX0; nX <= nX1; ++nX){
        for(int nY = nY0; nY <= nY1; ++nY){
            if(ValidC(nX, nY) && (LDistance2(nX, nY, stAMAS.X, stAMAS.Y) <= SYS_MAPVISIBLECD * SYS_MAPVISIBLECD)){
                for(auto pObject: m_ObjectV2D[nX][nY]){
                    if(pObject && pObject->Active()){
                        if(((ActiveObject *)(pObject))->Type(TYPE_HUMAN)){
                            m_ActorPod->Forward({MPK_ACTIONSTATE, stAMAS}, ((ActiveObject *)(pObject))->GetAddress());
                        }
                    }
                }
            }
        }
    }
}

void ServerMap::On_MPK_UPDATECOINFO(const MessagePack &rstMPK, const Theron::Address &)
{
    AMUpdateCOInfo stAMUCOI;
    std::memcpy(&stAMUCOI, rstMPK.Data(), sizeof(stAMUCOI));

    int nX0 = (stAMUCOI.X - SYS_MAPVISIBLECD);
    int nX1 = (stAMUCOI.X + SYS_MAPVISIBLECD);
    int nY0 = (stAMUCOI.Y - SYS_MAPVISIBLECD);
    int nY1 = (stAMUCOI.Y + SYS_MAPVISIBLECD);

    for(int nX = nX0; nX <= nX1; ++nX){
        for(int nY = nY0; nY <= nY1; ++nY){
            if(ValidC(nX, nY) && (LDistance2(nX, nY, stAMUCOI.X, stAMUCOI.Y) <= SYS_MAPVISIBLECD * SYS_MAPVISIBLECD)){
                for(auto pObject: m_ObjectV2D[nX][nY]){
                    if(pObject && pObject->Active()){
                        if(((ActiveObject *)(pObject))->Type(TYPE_HUMAN)){
                            m_ActorPod->Forward({MPK_UPDATECOINFO, stAMUCOI}, ((ActiveObject *)(pObject))->GetAddress());
                        }
                    }
                }
            }
        }
    }
}

void ServerMap::On_MPK_ADDCHAROBJECT(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMAddCharObject stAMACO;
    std::memcpy(&stAMACO, rstMPK.Data(), sizeof(stAMACO));

    if(!In(stAMACO.Common.MapID, stAMACO.Common.MapX, stAMACO.Common.MapY)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid argument in adding char object package");
        g_MonoServer->Restart();
    }

    if(GroundValid(stAMACO.Common.MapX, stAMACO.Common.MapY)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    if(m_CellState[stAMACO.Common.MapX][stAMACO.Common.MapY].Freezed){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    switch(stAMACO.Type){
        case TYPE_MONSTER:
            {
                auto pCO = new Monster(stAMACO.Monster.MonsterID,
                        m_ServiceCore,
                        this,
                        stAMACO.Common.MapX,
                        stAMACO.Common.MapY,
                        0,
                        STATE_INCARNATED,
                        STATE_STAND);
                pCO->Activate();
                m_ObjectV2D[stAMACO.Common.MapX][stAMACO.Common.MapY].push_back(pCO);
                m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
                break;
            }
        case TYPE_PLAYER:
            {
                auto pCO = new Player(stAMACO.Player.GUID,
                        stAMACO.Player.JobID,
                        0,
                        m_ServiceCore,
                        this,
                        stAMACO.Common.MapX,
                        stAMACO.Common.MapY,
                        0,
                        STATE_INCARNATED,
                        STATE_STAND);
                pCO->Activate();
                m_ActorPod->Forward({MPK_BINDSESSION, stAMACO.Player.SessionID}, pCO->GetAddress());
                m_ObjectV2D[stAMACO.Common.MapX][stAMACO.Common.MapY].push_back(pCO);
                m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
                break;
            }
        default:
            {
                // no idea what you want to create
                // return ERROR directly
                m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            }
    }
}

void ServerMap::On_MPK_TRYSPACEMOVE(const MessagePack &rstMPK, const Theron::Address &)
{
    AMTrySpaceMove stAMTSM;
    std::memcpy(&stAMTSM, rstMPK.Data(), sizeof(stAMTSM));

    if(!In(stAMTSM.MapID, stAMTSM.X, stAMTSM.Y)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "destination is not in current map, routing error");
        g_MonoServer->Restart();
    }
}

void ServerMap::On_MPK_TRYMOVE(const MessagePack &rstMPK, const Theron::Address &)
{
    AMTryMove stAMTM;
    std::memcpy(&stAMTM, rstMPK.Data(), sizeof(stAMTM));

    if(!In(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "destination is not in current map, routing error");
        g_MonoServer->Restart();
    }

    // // 0-0. reject any move request
    // if(m_MoveRequest.Freezed()){
    //     m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
    //     return;
    // }
    //
    // AMTryMove stAMTM;
    // std::memcpy(&stAMTM, rstMPK.Data(), sizeof(stAMTM));
    //
    // // 0-1. verify the package, for AMTryMove, we don't put current position inside since we assume
    // //      it should be in current RM, but we need to check the record v for (UID, AddTime)
    // if(!(stAMTM.MapID && stAMTM.X >= 0 && stAMTM.Y >= 0 && stAMTM.R >= 0)){
    //     extern MonoServer *g_MonoServer;
    //     g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid argument in try move request package");
    //     g_MonoServer->Restart();
    // }
    //
    // if(!(m_MapID && m_MapAddress)){
    //     extern MonoServer *g_MonoServer;
    //     g_MonoServer->AddLog(LOGTYPE_WARNING, "region monitor activated with invalid state");
    //     g_MonoServer->Restart();
    // }
    //
    // bool bFind = false;
    // for(auto &rstRecord: m_CORecordV){
    //     if(rstRecord.UID == stAMTM.UID && rstRecord.AddTime == stAMTM.AddTime){
    //         bFind = true;
    //         break;
    //     }
    // }
    //
    // if(!bFind){
    //     extern MonoServer *g_MonoServer;
    //     g_MonoServer->AddLog(LOGTYPE_WARNING, "can't find char object record in current RM, error in routing");
    //     g_MonoServer->Restart();
    // }
    //
    // // 0-2. define the lambda to send address string, will be used many times
    // auto fnSendRMAddr = [this, rstFromAddr, nMPKID = rstMPK.ID()](int nQuery, const Theron::Address & stRMAddr){
    //     switch(nQuery){
    //         case QUERY_OK:
    //             {
    //                 if(stRMAddr){
    //                     std::string szAddr = stRMAddr.AsString();
    //                     m_ActorPod->Forward({MPK_ADDRESS, (uint8_t *)szAddr.c_str(), 1 + szAddr.size()}, rstFromAddr, nMPKID);
    //                     return;
    //                 }
    //                 extern MonoServer *g_MonoServer;
    //                 g_MonoServer->AddLog(LOGTYPE_WARNING, "inconsistant logic: invalid argument for lambda");
    //                 g_MonoServer->Restart();
    //                 break;
    //             }
    //         case QUERY_PENDING:
    //             {
    //                 // the rm address is pending and we just report it
    //                 // TODO: I decide to obey the pending rule, always
    //                 m_ActorPod->Forward(MPK_PENDING, rstFromAddr, nMPKID);
    //                 break;
    //             }
    //         case QUERY_ERROR:
    //             {
    //                 // the rm queried is not valid, not capable to hold co
    //                 m_ActorPod->Forward(MPK_ERROR, rstFromAddr, nMPKID);
    //                 break;
    //             }
    //         case QUERY_NA:
    //         default:
    //             {
    //                 extern MonoServer *g_MonoServer;
    //                 g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported query state");
    //                 g_MonoServer->Restart();
    //                 break;
    //             }
    //     }
    // };
    //
    // // 1-0. check whether the reqestor is in void state
    // bool bVoidState = (stAMTM.R == 0);
    //
    // // 1-1. yes it's in void state
    // if(bVoidState){
    //     // 1-1-1. for void state, In() is good enough, we don't need neighbor's permission since
    //     //        the R now is zero, couldn't affect any neighbor
    //     if(In(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
    //         // even it's void state, we should check current requested point is valid
    //         if(!GroundValid(stAMTM.X, stAMTM.Y, stAMTM.R)){
    //             m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
    //             return;
    //         }
    //
    //         // ok current point is valid
    //         auto fnOnR = [this, stAMTM](const MessagePack &rstRMPK, const Theron::Address &){
    //             // 1. clear the lock
    //             m_MoveRequest.Clear();
    //             // 2. update the record
    //             if(rstRMPK.Type() == MPK_OK){
    //                 for(auto &rstRecord: m_CORecordV){
    //                     if(rstRecord.UID == stAMTM.UID && rstRecord.AddTime == stAMTM.AddTime){
    //                         rstRecord.X = stAMTM.X;
    //                         rstRecord.Y = stAMTM.Y;
    //                         rstRecord.R = 0;
    //
    //                         // TODO: here we don't check automatically whether the co still
    //                         //       need to be void state, the co should send void check
    //                         //       request sperately and by itself
    //                         return;
    //                     }
    //                 }
    //
    //                 // otherwise this should be an error
    //                 extern MonoServer *g_MonoServer;
    //                 g_MonoServer->AddLog(LOGTYPE_WARNING, "couldn't find this char object record");
    //                 g_MonoServer->Restart();
    //             }else{
    //                 // co decide to not move
    //                 // do nothing except unlock current RM
    //             }
    //
    //         };
    //
    //         m_MoveRequest.Clear();
    //         m_MoveRequest.WaitCO = true;
    //         m_MoveRequest.Freeze();
    //
    //         // permit this request, and wait for co's commit
    //         m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnOnR);
    //         return;
    //     }
    //
    //     // 1-1-2. not in current RM but in neighbor?
    //     if(NeighborIn(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
    //         // return the RM address
    //         // this won't return (1, 1) since we have shorted it by checking In()
    //         auto rstNRMAddr = NeighborAddress(stAMTM.X, stAMTM.Y);
    //         fnSendRMAddr(rstNRMAddr ? QUERY_OK : QUERY_ERROR, rstNRMAddr);
    //         return;
    //     }
    //
    //     // 1-1-3. even not in neighbor, this is a space move request, ooops it's annoying
    //     //        I use GetRMAddress() function to register state hooks
    //     QueryRMAddress(stAMTM.MapID, stAMTM.X, stAMTM.Y, false, fnSendRMAddr);
    //     return;
    // }
    //
    // // 2-0. ok it's not void state
    //
    // // 2-1. only in current RM, best situation
    // if(OnlyIn(stAMTM.MapID, stAMTM.X, stAMTM.Y, stAMTM.R)){
    //     // unlucky, this cover is not valid
    //     if(!CoverValid(stAMTM.UID, stAMTM.AddTime, stAMTM.X, stAMTM.Y, stAMTM.R)){
    //         m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
    //         return;
    //     }
    //
    //     // ok valid, prepare for locking
    //     m_MoveRequest.Clear();
    //     m_MoveRequest.WaitCO = true;
    //     m_MoveRequest.Freeze();
    //
    //     auto fnROP = [this, stAMTM](const MessagePack &rstRMPK, const Theron::Address &){
    //         // object moved, so we need to update the location
    //         if(rstRMPK.Type() == MPK_OK){
    //             for(auto &rstRecord: m_CORecordV){
    //                 if(rstRecord.UID == stAMTM.UID && rstRecord.AddTime == stAMTM.AddTime){
    //                     rstRecord.X = stAMTM.X;
    //                     rstRecord.Y = stAMTM.Y;
    //                     break;
    //                 }
    //             }
    //         }else{
    //             // object doesn't move actually
    //             // it gives up the chance to move, do nothing
    //         }
    //
    //         // no matter moved or not, release the current RM
    //         m_MoveRequest.Clear();
    //     };
    //
    //     m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnROP);
    //     return;
    // }
    //
    // // 2. in current RM but overlap with neighbors
    // if(In(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
    //     // unlucky, this cover is not valid
    //     if(!CoverValid(stAMTM.UID, stAMTM.AddTime, stAMTM.X, stAMTM.Y, stAMTM.R)){
    //         m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
    //         return;
    //     }
    //
    //     // ok valid, prepare for locking
    //
    //     // some needed neighbors are not qualified
    //     if(!NeighborValid(stAMTM.X, stAMTM.Y, stAMTM.R, true)){
    //         m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
    //         return;
    //     }
    //
    //     // ok now we need to prepare for the move request
    //
    //     // 01. now we are to do neighbor check
    //     m_MoveRequest.Clear();
    //     m_MoveRequest.NeighborCheck = true;
    //     m_MoveRequest.Freeze();
    //
    //     // 02. send cover check to neighbors
    //     NeighborSendCheck(stAMTM.UID, stAMTM.AddTime, stAMTM.X, stAMTM.Y, stAMTM.R, true);
    //
    //     // 03. make sure there is only one move request working
    //     if(m_StateHook.Installed("MoveRequest")){
    //         extern MonoServer *g_MonoServer;
    //         g_MonoServer->AddLog(LOGTYPE_WARNING, "try to install move request trigger while there's still one unfinished");
    //         g_MonoServer->Restart();
    //     }
    //
    //     // 03. install trigger to check the CC response
    //     auto fnMoveRequest = [this, stAMTM, rstFromAddr, nMPKID = rstMPK.ID()]() -> bool{
    //         switch(NeighborQueryCheck()){
    //             case QUERY_OK:
    //                 {
    //                     // we grant permission and now it's time
    //                     auto fnROP = [this, stAMTM](const MessagePack &rstRMPK, const Theron::Address &){
    //                         if(rstRMPK.Type() == MPK_OK){
    //                             // object picked this chance to move
    //                             for(auto &rstRecord: m_CORecordV){
    //                                 if(rstRecord.UID == stAMTM.UID && rstRecord.AddTime == stAMTM.AddTime){
    //                                     rstRecord.X = stAMTM.X;
    //                                     rstRecord.Y = stAMTM.Y;
    //                                     break;
    //                                 }
    //                             }
    //                         }
    //
    //                         // no matter the object decide to move or not, we need to free neighbors
    //                         NeighborClearCheck();
    //                         m_MoveRequest.Clear();
    //                     };
    //                     // TODO
    //                     // there was a bug here
    //                     // when we notified the object, then neighbor check is done
    //                     // however before the object responded, this RM should still be freezed
    //                     //
    //                     m_MoveRequest.Clear();
    //                     m_MoveRequest.WaitCO = true;
    //                     m_MoveRequest.Freeze();
    //
    //                     m_ActorPod->Forward(MPK_OK, rstFromAddr, nMPKID, fnROP);
    //                     return true;
    //                 }
    //             case QUERY_ERROR:
    //                 {
    //                     // no matter the object decide to move or not, we need to free neighbors
    //                     NeighborClearCheck();
    //                     m_MoveRequest.Clear();
    //
    //                     m_ActorPod->Forward(MPK_ERROR, rstFromAddr, nMPKID);
    //                     return true;
    //                 }
    //             case QUERY_PENDING:
    //                 {
    //                     // didn't finish it yet
    //                     return false;
    //                 }
    //             default:
    //                 {
    //                     extern MonoServer *g_MonoServer;
    //                     g_MonoServer->AddLog(LOGTYPE_WARNING, "internal logic error: unexcepted query status");
    //                     g_MonoServer->Restart();
    //                     // make the compiler happy
    //                     return false;
    //                 }
    //         }
    //
    //         // to make the compiler happy
    //         return false;
    //     };
    //
    //     m_StateHook.Install("MoveRequest", fnMoveRequest);
    //     return;
    // }
    //
    // // trying to move to one of its neighbor
    // // this is already a space move but we don't need to query the RM address since we have it
    //
    // if(NeighborIn(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
    //     // return the RM address
    //     // this won't return (1, 1) since we have shorted it by checking In()
    //     auto rstNRMAddr = NeighborAddress(stAMTM.X, stAMTM.Y);
    //     fnSendRMAddr(rstNRMAddr ? QUERY_OK : QUERY_ERROR, rstNRMAddr);
    //     return;
    // }
    //
    // // now this move request has nothing to do with current RM, then it's a space move, we have 
    // // to know the destination RM address, we design a logic which can take care of it
    //
    // QueryRMAddress(stAMTM.MapID, stAMTM.X, stAMTM.Y, false, fnSendRMAddr);
}

void ServerMap::On_MPK_LEAVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMLeave stAML;
    std::memcpy(&stAML, rstMPK.Data(), rstMPK.DataLen());

    bool bFind = false;
    auto &rstObjectV = m_ObjectV2D[stAML.X][stAML.Y];

    for(auto pObject: rstObjectV){
        if((uintptr_t)(pObject) == (uintptr_t)(stAML.This)){
            // 1. mark as find
            bFind = true;

            // 2. remove from the object list
            std::swap(rstObjectV.back(), pObject);
            rstObjectV.pop_back();

            // 3. inform the co that now you can leave
            m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
            break;
        }
    }

    if(!bFind){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_FATAL, "char object %p is not in current map", stAML.This);
        g_MonoServer->Restart();
    }
}
