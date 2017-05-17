/*
 * =====================================================================================
 *
 *       Filename: playernet.cpp
 *        Created: 05/19/2016 15:26:25
 *  Last Modified: 05/15/2017 14:05:58
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
    CMAction stCMA;
    std::memcpy(&stCMA, pBuf, sizeof(stCMA));

    if(true
            && stCMA.UID   == UID()
            && stCMA.MapID == MapID()

            && m_Map
            && m_Map->ValidC(stCMA.X, stCMA.Y)
            && m_Map->ValidC(stCMA.EndX, stCMA.EndY)){
        switch((int)(stCMA.Action)){
            case ACTION_MOVE:
                {
                    // server won't do any path finding
                    // client should sent action with only one-hop movement
                    switch(LDistance2(stCMA.X, stCMA.Y, stCMA.EndX, stCMA.EndY)){
                        case 1:
                        case 2:
                            {
                                break;
                            }
                        default:
                            {
                                extern MonoServer *g_MonoServer;
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid CMAction::UID         = %d", (int)(stCMA.UID));
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid CMAction::MapID       = %d", (int)(stCMA.MapID));
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid CMAction::Action      = %d", (int)(stCMA.Action));
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid CMAction::ActionParam = %d", (int)(stCMA.ActionParam));
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid CMAction::Speed       = %d", (int)(stCMA.Speed));
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid CMAction::Direction   = %d", (int)(stCMA.Direction));
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid CMAction::X           = %d", (int)(stCMA.X));
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid CMAction::Y           = %d", (int)(stCMA.Y));
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid CMAction::EndX        = %d", (int)(stCMA.EndX));
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid CMAction::EndY        = %d", (int)(stCMA.EndY));
                                return;
                            }
                    }

                    // OK the action is a valid one-hop motion
                    // we should check if the player can apply it
                    //
                    // 1. if apples, apply it without response, but dispatch it to neighbors
                    // 2. else send the pull back message

                    switch(LDistance2(X(), Y(), (int)(stCMA.X), (int)(stCMA.Y))){
                        case 0:
                            {
                                RequestMove((int)(stCMA.ActionParam), (int)(stCMA.EndX), (int)(stCMA.EndY), false, [](){}, [this](){ ReportStand(); });
                                return;
                            }
                        case 1:
                        case 2:
                            {
                                // there is one hop delay, acceptable
                                // try to do the one-hop and then try the client action if possible
                                auto fnOnFirstMoveOK = [this, stCMA](){
                                    RequestMove((int)(stCMA.ActionParam), (int)(stCMA.EndX), (int)(stCMA.EndY), false, [](){}, [this](){ ReportStand(); });
                                };

                                RequestMove((int)(stCMA.ActionParam), (int)(stCMA.X), (int)(stCMA.Y), false, fnOnFirstMoveOK, [this](){ ReportStand(); });
                                return;
                            }
                        default:
                            {
                                // difference is not acceptable
                                // force the client to do pull-back
                                ReportStand();
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
