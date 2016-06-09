/*
 * =====================================================================================
 *
 *       Filename: regionmonitorop.cpp
 *        Created: 05/03/2016 19:59:02
 *  Last Modified: 06/08/2016 23:14:01
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
#include "actorpod.hpp"
#include "charobject.hpp"
#include "monoserver.hpp"
#include "reactobject.hpp"
#include "regionmonitor.hpp"

void RegionMonitor::On_MPK_LEAVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMLeave stAML;
    std::memcpy(&stAML, rstMPK.Data(), rstMPK.DataLen());

    bool bFind = false;
    for(auto &rstRecord: m_CORecordV){
        if(rstRecord.UID == stAML.UID && rstRecord.AddTime == stAML.AddTime){
            std::swap(rstRecord, m_CORecordV.back());
            m_CORecordV.pop_back();
            bFind = true;
            break;
        }
    }

    if(!bFind){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "char object uid = %d, addtime = %d is not in current RM", stAML.UID, stAML.AddTime);
        g_MonoServer->Restart();
    }

    // commit the leave, then the object can move into another RM
    m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
}

void RegionMonitor::On_MPK_CHECKCOVER(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    // 1. there is working in current RM
    if(m_MoveRequest.Freezed()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // 2. ok let's do it
    AMCheckCover stAMCC;
    std::memcpy(&stAMCC, rstMPK.Data(), sizeof(stAMCC));

    // 3. checking failed, just repot and do nothing
    if(!CoverValid(stAMCC.UID, stAMCC.AddTime, stAMCC.X, stAMCC.Y, stAMCC.R)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // 3. checking succeeds
    //    we need to hold current RM till we receive notification
    //
    //    TODO: do I need to add a timeout functionality here??
    //          when time is up and we didn't get unlock notification, just report
    //          as error or automatically unlock it?
    m_MoveRequest.Clear();
    m_MoveRequest.CoverCheck = true;
    m_MoveRequest.Freeze();

    auto fnROP = [this](const MessagePack &, const Theron::Address &){
        // cover check requestor should response to clear the lock
        m_MoveRequest.Clear();
    };
    m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnROP);
}

void RegionMonitor::On_MPK_NEIGHBOR(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    // 1. parse the data
    char *pAddr = (char *)rstMPK.Data();
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            size_t nLen = std::strlen(pAddr);
            if(nLen == 0 || (nX == 1 && nY == 1)){
                m_NeighborV2D[nY][nX].PodAddress = Theron::Address::Null();
            }else{
                m_NeighborV2D[nY][nX].PodAddress = Theron::Address(pAddr);
            }
            pAddr += (1 + nLen);
        }
    }

    // 2. init the center pod for NeighborAddress()
    m_NeighborV2D[1][1].PodAddress = m_ActorPod->GetAddress();

    // 3. report finished to server map
    m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
}

void RegionMonitor::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    for(auto &rstRecord: m_CORecordV){
        m_ActorPod->Forward(MPK_METRONOME, rstRecord.PodAddress);
    }
}

void RegionMonitor::On_MPK_TRYMOVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    // TODO: do I need to lock RM if WaitCO == true && bVoidState == true ?
    //       if not, when waiting a void co's response, there could be new move request comes
    //       in, then there could be hairy logic since there are now more than one move
    //       request working

    // 0-0. reject any move request
    if(m_MoveRequest.Freezed()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    AMTryMove stAMTM;
    std::memcpy(&stAMTM, rstMPK.Data(), sizeof(stAMTM));

    // 0-1. verify the package, for AMTryMove, we don't put current position inside since we assume
    //      it should be in current RM, but we need to check the record v for (UID, AddTime)
    if(!(stAMTM.MapID && stAMTM.X >= 0 && stAMTM.Y >= 0 && stAMTM.R >= 0)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid argument in try move request package");
        g_MonoServer->Restart();
    }

    if(!(m_MapID && m_MapAddress)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "region monitor activated with invalid state");
        g_MonoServer->Restart();
    }

    bool bFind = false;
    for(auto &rstRecord: m_CORecordV){
        if(rstRecord.UID == stAMTM.UID && rstRecord.AddTime == stAMTM.AddTime){
            bFind = true;
            break;
        }
    }

    if(!bFind){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "can't find char object record in current RM, error in routing");
        g_MonoServer->Restart();
    }

    // 0-2. define the lambda to send address string, will be used many times
    auto fnSendRMAddr = [this, rstFromAddr, nMPKID = rstMPK.ID()](const Theron::Address & stRMAddr){
        if(stRMAddr){
            std::string szAddr = stRMAddr.AsString();
            m_ActorPod->Forward({MPK_ADDRESS, (uint8_t *)szAddr.c_str(), 1 + szAddr.size()}, rstFromAddr, nMPKID);
        }else{
            // we report error if with empty address, so if other logic is needed
            // check stRMAddr before invoke this lambda
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, nMPKID);
        }
    };

    // 1-0. check whether the reqestor is in void state
    bool bVoidState = (stAMTM.R == 0);

    // 1-1. yes it's in void state
    if(bVoidState){
        // 1-1-1. for void state, In() is good enough, we don't need neighbor's permission since
        //        the R now is zero, couldn't affect any neighbor
        if(In(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
            // even it's void state, we should check current requested point is valid
            if(!GroundValid(stAMTM.X, stAMTM.Y, stAMTM.R)){
                m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                return;
            }

            // ok current point is valid
            auto fnOnR = [this, stAMTM](const MessagePack &rstRMPK, const Theron::Address &){
                // 1. clear the lock
                m_MoveRequest.Clear();
                // 2. update the record
                if(rstRMPK.Type() == MPK_OK){
                    for(auto &rstRecord: m_CORecordV){
                        if(rstRecord.UID == stAMTM.UID && rstRecord.AddTime == stAMTM.AddTime){
                            rstRecord.X = stAMTM.X;
                            rstRecord.Y = stAMTM.Y;
                            rstRecord.R = 0;

                            // TODO: here we don't check automatically whether the co still
                            //       need to be void state, the co should send void check
                            //       request sperately and by itself
                            return;
                        }
                    }

                    // otherwise this should be an error
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "couldn't find this char object record");
                    g_MonoServer->Restart();
                }else{
                    // co decide to not move
                    // do nothing except unlock current RM
                }

            };

            m_MoveRequest.Clear();
            m_MoveRequest.WaitCO = true;
            m_MoveRequest.Freeze();

            // permit this request, and wait for co's commit
            m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnOnR);
            return;
        }

        // 1-1-2. not in current RM but in neighbor?
        if(NeighborIn(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
            // return the RM address
            // couldn't return (1, 1) since we have checked In()
            // fnSendRMAddr() send MPK_ERROR to co, means this neighbor is not capable to hoid the co
            fnSendRMAddr(NeighborAddress(stAMTM.X, stAMTM.Y));
            return;
        }

        // 1-1-3. even not in neighbor, this is a space move request, ooops it's annoying
        //        I use GetRMAddress() function to register state hooks
        QueryRMAddress(stAMTM.MapID, stAMTM.X, stAMTM.Y, fnSendRMAddr);
        return;
    }

    // 2-0. ok it's not void state

    // 2-1. only in current RM, best situation
    if(OnlyIn(stAMTM.MapID, stAMTM.X, stAMTM.Y, stAMTM.R)){
        // unlucky, this cover is not valid
        if(!CoverValid(stAMTM.UID, stAMTM.AddTime, stAMTM.X, stAMTM.Y, stAMTM.R)){
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            return;
        }

        // ok valid, prepare for locking
        m_MoveRequest.Clear();
        m_MoveRequest.WaitCO = true;
        m_MoveRequest.Freeze();

        auto fnROP = [this, stAMTM](const MessagePack &rstRMPK, const Theron::Address &){
            // object moved, so we need to update the location
            if(rstRMPK.Type() == MPK_OK){
                for(auto &rstRecord: m_CORecordV){
                    if(rstRecord.UID == stAMTM.UID && rstRecord.AddTime == stAMTM.AddTime){
                        rstRecord.X = stAMTM.X;
                        rstRecord.Y = stAMTM.Y;
                        break;
                    }
                }
            }else{
                // object doesn't move actually
                // it gives up the chance to move, do nothing
            }

            // no matter moved or not, release the current RM
            m_MoveRequest.Clear();
        };

        m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnROP);
        return;
    }

    // 2. in current RM but overlap with neighbors
    if(In(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
        // unlucky, this cover is not valid
        if(!CoverValid(stAMTM.UID, stAMTM.AddTime, stAMTM.X, stAMTM.Y, stAMTM.R)){
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            return;
        }

        // ok valid, prepare for locking

        // some needed neighbors are not qualified
        if(!NeighborValid(stAMTM.X, stAMTM.Y, stAMTM.R, true)){
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            return;
        }

        // ok now we need to prepare for the move request

        // 01. now we are to do neighbor check
        m_MoveRequest.Clear();
        m_MoveRequest.NeighborCheck = true;
        m_MoveRequest.Freeze();

        // 02. send cover check to neighbors
        NeighborSendCheck(stAMTM.UID, stAMTM.AddTime, stAMTM.X, stAMTM.Y, stAMTM.R, true);

        // 03. make sure there is only one move request working
        if(m_StateHook.Installed("MoveRequest")){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "try to install move request trigger while there's still one unfinished");
            g_MonoServer->Restart();
        }

        // 03. install trigger to check the CC response
        auto fnMoveRequest = [this, stAMTM, rstFromAddr, nMPKID = rstMPK.ID()]() -> bool{
            switch(NeighborQueryCheck()){
                case QUERY_OK:
                    {
                        // we grant permission and now it's time
                        auto fnROP = [this, stAMTM](const MessagePack &rstRMPK, const Theron::Address &){
                            if(rstRMPK.Type() == MPK_OK){
                                // object picked this chance to move
                                for(auto &rstRecord: m_CORecordV){
                                    if(rstRecord.UID == stAMTM.UID && rstRecord.AddTime == stAMTM.AddTime){
                                        rstRecord.X = stAMTM.X;
                                        rstRecord.Y = stAMTM.Y;
                                        break;
                                    }
                                }
                            }

                            // no matter the object decide to move or not, we need to free neighbors
                            NeighborClearCheck();
                            m_MoveRequest.Clear();
                        };
                        // TODO
                        // there was a bug here
                        // when we notified the object, then neighbor check is done
                        // however before the object responded, this RM should still be freezed
                        //
                        m_MoveRequest.Clear();
                        m_MoveRequest.WaitCO = true;
                        m_MoveRequest.Freeze();

                        m_ActorPod->Forward(MPK_OK, rstFromAddr, nMPKID, fnROP);
                        return true;
                    }
                case QUERY_ERROR:
                    {
                        // no matter the object decide to move or not, we need to free neighbors
                        NeighborClearCheck();
                        m_MoveRequest.Clear();

                        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, nMPKID);
                        return true;
                    }
                case QUERY_PENDING:
                    {
                        // didn't finish it yet
                        return false;
                    }
                default:
                    {
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "internal logic error: unexcepted query status");
                        g_MonoServer->Restart();
                        // make the compiler happy
                        return false;
                    }
            }

            // to make the compiler happy
            return false;
        };

        m_StateHook.Install("MoveRequest", fnMoveRequest);
        return;
    }

    // trying to move to one of its neighbor
    // this is already a space move but we don't need to query the RM address since we have it

    if(NeighborIn(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
        // return the RM address
        // couldn't return (1, 1) since we have checked In()
        // fnSendRMAddr() send MPK_ERROR to co, means this neighbor is not capable to hoid the co
        fnSendRMAddr(NeighborAddress(stAMTM.X, stAMTM.Y));
        return;
    }

    // now this move request has nothing to do with current RM, then it's a space move, we have 
    // to know the destination RM address, we design a logic which can take care of it

    QueryRMAddress(stAMTM.MapID, stAMTM.X, stAMTM.Y, fnSendRMAddr);
}

void RegionMonitor::On_MPK_TRYSPACEMOVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    // TODO: do I need to lock RM if WaitCO == true && bVoidState == true ?
    //       if not, when waiting a void co's response, there could be new move request comes
    //       in, then there could be hairy logic since there are now more than one move
    //       request working

    // 0-0. reject any move request
    if(m_MoveRequest.Freezed()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    AMTrySpaceMove stAMTSM;
    std::memcpy(&stAMTSM, rstMPK.Data(), sizeof(stAMTSM));

    // 0-1. verify the package, for AMTryMove, we don't put current position inside since we assume
    //      it should be in current RM, but we need to check the record v for (UID, AddTime)
    if(!(stAMTSM.MapID && stAMTSM.X >= 0 && stAMTSM.Y >= 0 && stAMTSM.CurrX >= 0 && stAMTSM.CurrY >= 0 && stAMTSM.R >= 0)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid argument in try space move request package");
        g_MonoServer->Restart();
    }

    if(!(m_MapID && m_MapAddress)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "region monitor activated with invalid state");
        g_MonoServer->Restart();
    }

    bool bFind = false;
    for(auto &rstRecord: m_CORecordV){
        if(rstRecord.UID == stAMTSM.UID && rstRecord.AddTime == stAMTSM.AddTime){
            bFind = true;
            break;
        }
    }

    if(bFind){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "find char object record in current RM, error in space move routing");
        g_MonoServer->Restart();
    }

    if(In(stAMTSM.MapID, stAMTSM.CurrX, stAMTSM.CurrY)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "destination is the same as staring point, routing error");
        g_MonoServer->Restart();
    }

    if(!In(stAMTSM.MapID, stAMTSM.X, stAMTSM.Y)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "destination is not in current RM, routing error");
        g_MonoServer->Restart();
    }

    // 1-0. check whether the reqestor is in void state
    bool bVoidState = (stAMTSM.R == 0);

    // 1-1. yes it's in void state
    if(bVoidState){
        // 1-1-1. for void state, In() is good enough, we don't need neighbor's permission since
        //        the R now is zero, couldn't affect any neighbor
        if(In(stAMTSM.MapID, stAMTSM.X, stAMTSM.Y)){
            // even it's void state, we should check current requested point is valid
            if(!GroundValid(stAMTSM.X, stAMTSM.Y, stAMTSM.R)){
                m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                return;
            }

            // ok current point is valid
            auto fnOnR = [this, stAMTSM](const MessagePack &rstRMPK, const Theron::Address &rstAddr){
                // 1. clear the lock
                m_MoveRequest.Clear();
                // 2. update the record
                if(rstRMPK.Type() == MPK_OK){
                    CORecord stCORecord;
                    stCORecord.X = stAMTSM.X;
                    stCORecord.Y = stAMTSM.Y;
                    stCORecord.R = stAMTSM.R;

                    stCORecord.UID        = stAMTSM.UID;
                    stCORecord.AddTime    = stAMTSM.AddTime;
                    stCORecord.PodAddress = rstAddr;

                    // put the new record into current region
                    m_CORecordV.push_back(stCORecord);

                    // TODO: here we don't check automatically whether the co still
                    //       need to be void state, the co should send void check
                    //       request sperately and by itself
                }
            };

            m_MoveRequest.Clear();
            m_MoveRequest.WaitCO = true;
            m_MoveRequest.Freeze();

            // permit this request, and wait for co's commit
            m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnOnR);
            return;
        }

        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "destination is not in current RM, routing error");
        g_MonoServer->Restart();
    }

    // 2-0. ok it's not void state

    // 2-1. only in current RM, best situation
    if(OnlyIn(stAMTSM.MapID, stAMTSM.X, stAMTSM.Y, stAMTSM.R)){
        // unlucky, this cover is not valid
        if(!CoverValid(stAMTSM.UID, stAMTSM.AddTime, stAMTSM.X, stAMTSM.Y, stAMTSM.R)){
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            return;
        }

        // ok valid, prepare for locking
        m_MoveRequest.Clear();
        m_MoveRequest.WaitCO = true;
        m_MoveRequest.Freeze();

        auto fnROP = [stAMTSM, this](const MessagePack &rstRMPK, const Theron::Address &){
            // object moved, so we need to update the location
            if(rstRMPK.Type() == MPK_OK){
                for(auto &rstRecord: m_CORecordV){
                    if(rstRecord.UID == stAMTSM.UID && rstRecord.AddTime == stAMTSM.AddTime){
                        rstRecord.X = stAMTSM.X;
                        rstRecord.Y = stAMTSM.Y;
                        break;
                    }
                }
            }else{
                // object doesn't move actually
                // it gives up the chance to move, do nothing
            }

            // no matter moved or not, release the current RM
            m_MoveRequest.Clear();
        };

        m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnROP);
        return;
    }

    // 2. in current RM but overlap with neighbors
    if(In(stAMTSM.MapID, stAMTSM.X, stAMTSM.Y)){
        // unlucky, this cover is not valid
        if(!CoverValid(stAMTSM.UID, stAMTSM.AddTime, stAMTSM.X, stAMTSM.Y, stAMTSM.R)){
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            return;
        }

        // ok valid, prepare for locking

        // some needed neighbors are not qualified
        if(!NeighborValid(stAMTSM.X, stAMTSM.Y, stAMTSM.R, true)){
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            return;
        }

        // ok now we need to prepare for the move request

        // 01. now we are to do neighbor check
        m_MoveRequest.Clear();
        m_MoveRequest.NeighborCheck = true;
        m_MoveRequest.Freeze();

        // 02. send cover check to neighbors
        NeighborSendCheck(stAMTSM.UID, stAMTSM.AddTime, stAMTSM.X, stAMTSM.Y, stAMTSM.R, true);

        // 03. make sure there is only one move request working
        if(m_StateHook.Installed("MoveRequest")){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "try to install move request trigger while there's still one unfinished");
            g_MonoServer->Restart();
        }

        // 03. install trigger to check the CC response
        auto fnMoveRequest = [this, stAMTSM, rstFromAddr, nMPKID = rstMPK.ID()]() -> bool {
            switch(NeighborQueryCheck()){
                case QUERY_OK:
                    {
                        // we grant permission and now it's time
                        auto fnROP = [stAMTSM, this](const MessagePack &rstRMPK, const Theron::Address &){
                            if(rstRMPK.Type() == MPK_OK){
                                // object picked this chance to move
                                for(auto &rstRecord: m_CORecordV){
                                    if(rstRecord.UID == stAMTSM.UID && rstRecord.AddTime == stAMTSM.AddTime){
                                        rstRecord.X = stAMTSM.X;
                                        rstRecord.Y = stAMTSM.Y;
                                        break;
                                    }
                                }
                            }

                            // no matter the object decide to move or not, we need to free neighbors
                            NeighborClearCheck();
                            m_MoveRequest.Clear();
                        };
                        // TODO
                        // there was a bug here
                        // when we notified the object, then neighbor check is done
                        // however before the object responded, this RM should still be freezed
                        //
                        m_MoveRequest.Clear();
                        m_MoveRequest.WaitCO = true;
                        m_MoveRequest.Freeze();

                        m_ActorPod->Forward(MPK_OK, rstFromAddr, nMPKID, fnROP);
                        return true;
                    }
                case QUERY_ERROR:
                    {
                        // no matter the object decide to move or not, we need to free neighbors
                        NeighborClearCheck();
                        m_MoveRequest.Clear();

                        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, nMPKID);
                        return true;
                    }
                case QUERY_PENDING:
                    {
                        // didn't finish it yet
                        return false;
                    }
                default:
                    {
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "internal logic error: unexcepted query status");
                        g_MonoServer->Restart();
                        // make the compiler happy
                        return false;
                    }
            }

            // to make the compiler happy
            return false;
        };

        m_StateHook.Install("MoveRequest", fnMoveRequest);
        return;
    }

    // outside of current RM, should be an error
    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "destination is not in current rm, routing error");
    g_MonoServer->Restart();
}

void RegionMonitor::On_MPK_ADDCHAROBJECT(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    if(m_MoveRequest.Freezed()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    AMAddCharObject stAMACO;
    std::memcpy(&stAMACO, rstMPK.Data(), sizeof(stAMACO));

    if(!(stAMACO.Common.MapID && stAMACO.Common.MapX >= 0 && stAMACO.Common.MapY >= 0 && stAMACO.Common.R >= 0)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid argument in adding char object package");
        g_MonoServer->Restart();
    }

    if(!(m_MapID && m_MapAddress)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "region monitor activated with invalid state");
        g_MonoServer->Restart();
    }

    if(!In(stAMACO.Common.MapID, stAMACO.Common.MapX, stAMACO.Common.MapY)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid argument in adding object package, routing error");
        g_MonoServer->Restart();
    }

    // declare lambda to make co and co record if needed
    auto fnCreateCO = [stAMACO](){
        CharObject *pCharObject = nullptr;
        switch(stAMACO.Type){
            case OBJECT_MONSTER:
                {
                    pCharObject = new Monster(stAMACO.Monster.MonsterID);
                    break;
                }
            case OBJECT_PLAYER:
                {
                    pCharObject = new Player(stAMACO.Player.GUID, stAMACO.Player.JobID);
                    break;
                }
            default:
                {
                    break;
                }
        }
        return pCharObject;
    };

    auto fnCreateCORecord = [this, rstFromAddr, fnCreateCO, stAMACO, nMPKID = rstMPK.ID()](bool bVoidState){
        CharObject *pCharObject = fnCreateCO();
        if(!pCharObject){
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, nMPKID);
            return;
        }

        // if it's only in then we can set it as active immediately
        pCharObject->ResetState(bVoidState ? STATE_INCARNATED : STATE_ACTIVE, true);
        pCharObject->ResetR(stAMACO.Common.R);
        pCharObject->Locate(m_MapID, stAMACO.Common.MapX, stAMACO.Common.MapY);
        pCharObject->Locate(GetAddress());

        CORecord stCORecord;
        stCORecord.X = pCharObject->X();
        stCORecord.Y = pCharObject->Y();
        stCORecord.R = pCharObject->R();

        stCORecord.UID        = pCharObject->UID();
        stCORecord.AddTime    = pCharObject->AddTime();
        stCORecord.PodAddress = pCharObject->Activate();

        m_CORecordV.push_back(stCORecord);

        // we respond to ServiceCore, but it won't respond again
        // TODO & TBD
        // here maybe protential bug:
        // 1. message to creater did arrival yet
        // 2. but the actor has already been working
        //
        m_ActorPod->Forward(MPK_OK, rstFromAddr, nMPKID);

        // actually here we don't need to create the RM address and send it
        // since for the receiving object, it can take the address of the Operate()
        m_ActorPod->Forward(MPK_HI, stCORecord.PodAddress);

        if(pCharObject->Type(OBJECT_PLAYER)){
            m_ActorPod->Forward({MPK_BINDSESSION, stAMACO.Player.SessionID}, stCORecord.PodAddress);
        }
    };

    // we use R to decide whether it's void or not
    // and after the adding, co should load the real R by itself
    bool bVoidState = (stAMACO.Common.R == 0);

    int nMapX = stAMACO.Common.MapX;
    int nMapY = stAMACO.Common.MapY;
    int nR    = stAMACO.Common.R;

    uint32_t nMapID = stAMACO.Common.MapID;

    // we can immediately reject it
    if(!(bVoidState || CoverValid(0, 0, nMapX, nMapY, nR))){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // this is the condition we can add the co immediately
    if(bVoidState){
        if(GroundValid(nMapX, nMapY, nR)){
            fnCreateCORecord(true);
            return;
        }

        // else just reject since the ground is not valid
        // can not satisfy the minimal requirement
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // not void state, so we need to check the cover validness
    if(!CoverValid(0, 0, nMapX, nMapY, nR)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // now at least cover check in current RM is ok

    if(OnlyIn(nMapID, nMapX, nMapY, nR)){
        fnCreateCORecord(false);
        return;
    }

    if(In(nMapID, nMapX, nMapY)){
        // neighbor validation
        if(!NeighborValid(nMapX, nMapY, nR, true)){
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            return;
        }

        // ok neighbor is valid, try to get their permission
        m_MoveRequest.Clear();
        m_MoveRequest.NeighborCheck = true;
        m_MoveRequest.Freeze();

        NeighborSendCheck(0, 0, nMapX, nMapY, nR, true);

        if(m_StateHook.Installed("AddCO")){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "try to add co while another adding process is taking place");
            g_MonoServer->Restart();
        }

        auto fnAddCO = [this, rstFromAddr, fnCreateCORecord, nMPKID = rstMPK.ID()]() -> bool {
            switch(NeighborQueryCheck()){
                case QUERY_OK:
                    {
                        fnCreateCORecord(false);
                        m_MoveRequest.Clear();
                        NeighborClearCheck();

                        return true;
                    }
                case QUERY_PENDING:
                    {
                        // do nothing
                        return false;
                    }
                case QUERY_ERROR:
                    {
                        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, nMPKID);
                        return true;
                    }
                default:
                    {
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "internal logic error, shouldn't be here");
                        g_MonoServer->Restart();
                        return false;
                    }
            }

            // to make the compiler happy
            return false;
        };

        m_StateHook.Install("AddCO", fnAddCO);
        return;
    }
    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "internal logic error, shouldn't be here");
    g_MonoServer->Restart();
}

void RegionMonitor::On_MPK_MOTIONSTATE(const MessagePack &rstMPK, const Theron::Address &)
{
    for(auto &rstCORecord: m_CORecordV){
        if(rstCORecord.Type == OBJECT_PLAYER && rstCORecord.PodAddress){
            m_ActorPod->Forward({MPK_MOTIONSTATE, rstMPK.Data(), rstMPK.DataLen()}, rstCORecord.PodAddress);
        }
    }
}

void RegionMonitor::On_MPK_QUERYSCADDRESS(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    if(m_SCAddress){
        std::string szAddr = m_SCAddress.AsString();
        m_ActorPod->Forward({MPK_ADDRESS, (const uint8_t *)(szAddr.c_str()), 1 + szAddr.size()}, rstFromAddr, rstMPK.ID());
        return;
    }

    // sc address is not valid, but it's on the way
    if(m_SCAddressQuery == QUERY_PENDING){
        auto fnResp = [this, rstFromAddr, rstMPK](){
            if(m_SCAddress){
                std::string szAddr = m_SCAddress.AsString();
                m_ActorPod->Forward({MPK_ADDRESS, (const uint8_t *)(szAddr.c_str()), 1 + szAddr.size()}, rstFromAddr, rstMPK.ID());
                return true;
            }

            return false;
        };

        m_StateHook.Install(fnResp);
        return;
    }

    // oooops we have no SC address, ask map for sc address
    if(m_MapAddress){
        auto fnOnR = [nMPKID = rstMPK.ID(), rstFromAddr, this](const MessagePack &rstRMPK, const Theron::Address &){
            switch(rstRMPK.Type()){
                case MPK_ADDRESS:
                    {
                        m_SCAddress = Theron::Address((const char *)rstRMPK.Data());
                        m_ActorPod->Forward({MPK_ADDRESS, rstRMPK.Data(), rstRMPK.DataLen()}, rstFromAddr, nMPKID);
                        break;
                    }
                default:
                    {
                        // sc address query failed, couldn't happen
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "query service core address failed, couldn't happen");
                        g_MonoServer->Restart();
                        break;
                    }
            }
        };
        m_ActorPod->Forward(MPK_QUERYSCADDRESS, m_MapAddress, fnOnR);
    }

    // otherwise it's should be an error
    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "activated RM works without valid map address");
    g_MonoServer->Restart();
}

void RegionMonitor::On_MPK_QUERYMAPADDRESS(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    auto nMapID = *((uint32_t *)rstMPK.Data());
    if(nMapID == m_MapID){
        if(m_MapAddress){
            std::string szMapAddr = m_MapAddress.AsString();
            m_ActorPod->Forward({MPK_ADDRESS, (const uint8_t *)(szMapAddr.c_str(), 1 + szMapAddr.size())}, rstFromAddr, rstMPK.ID());
            return;
        }

        // otherwise it's should be an error
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "activated RM works without valid map address");
        g_MonoServer->Restart();
    }

    // you should ask sc directly
    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "not RM's map address, ask service core for this instead");
    g_MonoServer->Restart();
}
