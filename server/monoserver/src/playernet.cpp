/*
 * =====================================================================================
 *
 *       Filename: playernet.cpp
 *        Created: 05/19/2016 15:26:25
 *  Last Modified: 06/13/2016 23:07:42
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

#include "player.hpp"
#include "message.hpp"
#include "actorpod.hpp"
#include "monoserver.hpp"
void Player::Net_CM_MOTION(uint8_t, const uint8_t *, size_t)
{
}

void Player::Net_CM_QUERYMONSTERGINFO(uint8_t, const uint8_t *pBuf, size_t)
{
    AMQueryMonsterGInfo stAMQMGI;
    stAMQMGI.MonsterID = ((CMQueryMonsterGInfo *)(pBuf))->MonsterID;
    stAMQMGI.LookIDN   = (int)((CMQueryMonsterGInfo *)(pBuf))->LookIDN;
    stAMQMGI.SessionID = m_SessionID;

    switch(QuerySCAddress()){
        case QUERY_OK:
            {
                if(m_SCAddress){
                    m_ActorPod->Forward({MPK_QUERYMONSTERGINFO, stAMQMGI}, m_SCAddress);
                    return;
                }

                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "unexpected internal error");
                g_MonoServer->Restart();
                break;
            }
        case QUERY_PENDING:
            {
                auto fnOnSCAddr = [this, stAMQMGI]() -> bool{
                    switch(QuerySCAddress()){
                        case QUERY_OK:
                            {
                                m_ActorPod->Forward({MPK_QUERYMONSTERGINFO, stAMQMGI}, m_SCAddress);
                                return true;
                            }
                        case QUERY_PENDING:
                            {
                                return false;
                            }
                        default:
                            {
                                extern MonoServer *g_MonoServer;
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "unexpected internal error");
                                g_MonoServer->Restart();
                                // to make the compiler happy
                                return false;
                            }
                    }
                    // to make the compiler happy
                    return false;
                };
                m_StateHook.Install(fnOnSCAddr);
                return;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "unexpected internal error");
                g_MonoServer->Restart();
                break;
            }
    }
}
