/*
 * =====================================================================================
 *
 *       Filename: playernet.cpp
 *        Created: 05/19/2016 15:26:25
 *  Last Modified: 07/02/2017 21:51:22
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
                    int nMaxStep = 0;
                    switch(stCMA.ActionParam){
                        case MOTION_WALK      : nMaxStep = 1; break;
                        case MOTION_RUN       : nMaxStep = 2; break;
                        case MOTION_HORSEWALK : nMaxStep = 1; break;
                        case MOTION_HORSERUN  : nMaxStep = 3; break;
                        default               : return;
                    }

                    int nDX = std::abs<int>(stCMA.EndX - stCMA.X);
                    int nDY = std::abs<int>(stCMA.EndY - stCMA.Y);

                    if(true
                            && (std::max<int>(nDX, nDY) == nMaxStep)
                            && (std::min<int>(nDX, nDY) == 0 || nDX == nDY)){
                    }else{
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

void Player::Net_CM_QUERYCORECORD(uint8_t, const uint8_t *pBuf, size_t)
{
    CMQueryCORecord stCMQCOR;
    std::memcpy(&stCMQCOR, pBuf, sizeof(stCMQCOR));

    if(true
            && stCMQCOR.UID
            && stCMQCOR.MapID == MapID()

            && m_Map
            && m_Map->ValidC(stCMQCOR.X, stCMQCOR.Y)
            && m_Map->ActorPodValid()){
        // 1. check cached actor address first
        // 2. then send to map
        AMQueryCORecord stAMQCOR;
        stAMQCOR.UID       = stCMQCOR.UID;
        stAMQCOR.MapID     = stCMQCOR.MapID;
        stAMQCOR.X         = stCMQCOR.X;
        stAMQCOR.Y         = stCMQCOR.Y;
        stAMQCOR.SessionID = SessionID();
        m_ActorPod->Forward({MPK_QUERYCORECORD, stAMQCOR}, m_Map->GetAddress());
    }
}
