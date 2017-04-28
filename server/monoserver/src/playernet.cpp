/*
 * =====================================================================================
 *
 *       Filename: playernet.cpp
 *        Created: 05/19/2016 15:26:25
 *  Last Modified: 04/27/2017 17:38:38
 *
 *    Description: how player respond for different net package
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

#include <cinttypes>

#include "player.hpp"
#include "message.hpp"
#include "actorpod.hpp"
#include "monoserver.hpp"

void Player::Net_CM_QUERYMONSTERGINFO(uint8_t, const uint8_t *pBuf, size_t)
{
    AMQueryMonsterGInfo stAMQMGI;
    stAMQMGI.MonsterID = ((CMQueryMonsterGInfo *)(pBuf))->MonsterID;
    stAMQMGI.LookIDN   = (int)((CMQueryMonsterGInfo *)(pBuf))->LookIDN;
    stAMQMGI.SessionID = m_SessionID;

    if(ActorPodValid() && m_ServiceCore && m_ServiceCore->ActorPodValid()){
        m_ActorPod->Forward({MPK_QUERYMONSTERGINFO, stAMQMGI}, m_ServiceCore->GetAddress());
        return;
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "unexpected internal error");
    g_MonoServer->Restart();
}

void Player::Net_CM_ACTION(uint8_t, const uint8_t *pBuf, size_t)
{
    SMAction stSMA;
    std::memcpy(&stSMA, pBuf, sizeof(stSMA));

    if(true
            && m_Map
            && m_Map->ValidC(stSMA.X, stSMA.Y)
            && m_Map->ValidC(stSMA.EndX, stSMA.EndY)){
        switch((int)(stSMA.Action)){
            case ACTION_MOVE:
                {
                    // server won't do any path finding
                    // client should sent action with only one-hop movement
                    switch(LDistance2(stSMA.X, stSMA.Y, stSMA.EndX, stSMA.EndY)){
                        case 1:
                        case 2:
                            {
                                break;
                            }
                        default:
                            {
                                extern MonoServer *g_MonoServer;
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid SMAction::UID         = %d", stSMA.UID);
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid SMAction::MapID       = %d", stSMA.MapID);
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid SMAction::Action      = %d", stSMA.Action);
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid SMAction::ActionParam = %d", stSMA.ActionParam);
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid SMAction::Speed       = %d", stSMA.Speed);
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid SMAction::Direction   = %d", stSMA.Direction);
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid SMAction::X           = %d", stSMA.X);
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid SMAction::Y           = %d", stSMA.Y);
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid SMAction::EndX        = %d", stSMA.EndX);
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid SMAction::EndY        = %d", stSMA.EndY);
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid SMAction::ID          = %" PRIu32, stSMA.ID);
                                return;
                            }
                    }

                    // OK the action is a valid one-hop motion
                    // we should check if the player can apply it
                    //
                    // 1. if apples, apply it without response, but dispatch it to neighbors
                    // 2. else send the pull back message

                    switch(LDistance2(X(), Y(), (int)(stSMA.X), (int)(stSMA.Y))){
                        case 0:
                            {
                                auto fnOnMoveOK = [](){
                                    // do nothing
                                };

                                auto fnOnMoveError = [this](){
                                    // failed for motion
                                    // send current stand status to client
                                    SMAction stSMActionStand;
                                    stSMActionStand.UID         = UID();
                                    stSMActionStand.MapID       = MapID();
                                    stSMActionStand.Action      = ACTION_STAND;
                                    stSMActionStand.ActionParam = 0;
                                    stSMActionStand.Speed       = 0;
                                    stSMActionStand.Direction   = Direction();
                                    stSMActionStand.X           = X();
                                    stSMActionStand.Y           = Y();
                                    stSMActionStand.EndX        = X();
                                    stSMActionStand.EndY        = Y();
                                    stSMActionStand.ID          = 0;

                                    extern NetPodN *g_NetPodN;
                                    g_NetPodN->Send(m_SessionID, SM_ACTION, stSMActionStand);
                                };
                                RequestMove((int)(stSMA.EndX), (int)(stSMA.EndY), fnOnMoveOK, fnOnMoveError);
                                return;
                            }
                        case 1:
                        case 2:
                            {
                                // there is one hop delay, acceptable
                                // try to do the one-hop and then try the client action if possible
                                auto fnOnFirstMoveOK = [this, stSMA](){
                                    auto fnOnSecondMoveOK = [](){
                                        // do nothing
                                    };

                                    auto fnOnSecondMoveError = [this](){
                                        // failed for motion
                                        // send current stand status to client
                                        SMAction stSMActionStand;
                                        stSMActionStand.UID         = UID();
                                        stSMActionStand.MapID       = MapID();
                                        stSMActionStand.Action      = ACTION_STAND;
                                        stSMActionStand.ActionParam = 0;
                                        stSMActionStand.Speed       = 0;
                                        stSMActionStand.Direction   = Direction();
                                        stSMActionStand.X           = X();
                                        stSMActionStand.Y           = Y();
                                        stSMActionStand.EndX        = X();
                                        stSMActionStand.EndY        = Y();
                                        stSMActionStand.ID          = 0;

                                        extern NetPodN *g_NetPodN;
                                        g_NetPodN->Send(m_SessionID, SM_ACTION, stSMActionStand);
                                    };
                                    RequestMove((int)(stSMA.EndX), (int)(stSMA.EndY), fnOnSecondMoveOK, fnOnSecondMoveError);
                                };

                                auto fnOnFirstMoveError = [this](){
                                    SMAction stSMActionStand;
                                    stSMActionStand.UID         = UID();
                                    stSMActionStand.MapID       = MapID();
                                    stSMActionStand.Action      = ACTION_STAND;
                                    stSMActionStand.ActionParam = 0;
                                    stSMActionStand.Speed       = 0;
                                    stSMActionStand.Direction   = Direction();
                                    stSMActionStand.X           = X();
                                    stSMActionStand.Y           = Y();
                                    stSMActionStand.EndX        = X();
                                    stSMActionStand.EndY        = Y();
                                    stSMActionStand.ID          = 0;

                                    extern NetPodN *g_NetPodN;
                                    g_NetPodN->Send(m_SessionID, SM_ACTION, stSMActionStand);
                                };

                                RequestMove((int)(stSMA.X), (int)(stSMA.Y), fnOnFirstMoveOK, fnOnFirstMoveError);
                                return;
                            }
                        default:
                            {
                                SMAction stSMActionStand;
                                stSMActionStand.UID         = UID();
                                stSMActionStand.MapID       = MapID();
                                stSMActionStand.Action      = ACTION_STAND;
                                stSMActionStand.ActionParam = 0;
                                stSMActionStand.Speed       = 0;
                                stSMActionStand.Direction   = Direction();
                                stSMActionStand.X           = X();
                                stSMActionStand.Y           = Y();
                                stSMActionStand.EndX        = X();
                                stSMActionStand.EndY        = Y();
                                stSMActionStand.ID          = 0;

                                extern NetPodN *g_NetPodN;
                                g_NetPodN->Send(m_SessionID, SM_ACTION, stSMActionStand);
                                return;
                            }
                    }
                    break;
                }
            case ACTION_STAND:
                {
                    break;
                }
            default:
                {
                    break;
                }
        }
    }
}
