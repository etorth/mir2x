/*
 * =====================================================================================
 *
 *       Filename: playernet.cpp
 *        Created: 05/19/2016 15:26:25
 *  Last Modified: 07/08/2017 14:12:08
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
            && m_Map->ValidC(stCMA.AimX, stCMA.AimY)){
        switch((int)(stCMA.Action)){
            case ACTION_MOVE:
                {
                    // server won't do any path finding
                    // client should sent action with only one-hop movement

                    int nMotionMode = -1;
                    switch(LDistance2(stCMA.X, stCMA.Y, stCMA.AimX, stCMA.AimY)){
                        case 1:
                        case 2:
                            {
                                nMotionMode = stCMA.ActionParam ? MOTION_ONHORSEWALK : MOTION_WALK;
                                break;
                            }
                        case 4:
                        case 8:
                            {
                                nMotionMode = MOTION_RUN;
                                break;
                            }
                        case  9:
                        case 18:
                            {
                                nMotionMode = MOTION_ONHORSERUN;
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
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid CMAction::AimX        = %d", (int)(stCMA.AimX));
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid CMAction::AimY        = %d", (int)(stCMA.AimY));
                                return;
                            }
                    }

                    // OK the action is a valid one-hop motion
                    // we should check if the player can apply it
                    //
                    // 1. if apples, apply it without response, but dispatch it to neighbors
                    // 2. else send the pull back message

                    int nX0 = (int)(stCMA.X);
                    int nY0 = (int)(stCMA.Y);
                    int nX1 = (int)(stCMA.AimX);
                    int nY1 = (int)(stCMA.AimY);

                    switch(LDistance2(X(), Y(), nX0, nY0)){
                        case 0:
                            {
                                RequestMove(nMotionMode, nX1, nY1, false, [](){}, [this](){ ReportStand(); });
                                return;
                            }
                        case  1:
                        case  2:
                        case  4:
                        case  8:
                        case  9:
                        case 18:
                            {
                                // there is one hop delay, acceptable
                                // try to do the one-hop and then try the client action if possible
                                auto fnOnFirstMoveOK = [this, nMotionMode, nX1, nY1](){
                                    RequestMove(nMotionMode, nX1, nY1, false, [](){}, [this](){ ReportStand(); });
                                };

                                RequestMove(nMotionMode, nX0, nY0, false, fnOnFirstMoveOK, [this](){ ReportStand(); });
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
