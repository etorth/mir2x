/*
 * =====================================================================================
 *
 *       Filename: playernet.cpp
 *        Created: 05/19/2016 15:26:25
 *  Last Modified: 03/22/2017 17:27:44
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

    if(ActorPodValid() && m_ServiceCore && m_ServiceCore->ActorPodValid()){
        m_ActorPod->Forward({MPK_QUERYMONSTERGINFO, stAMQMGI}, m_ServiceCore->GetAddress());
        return;
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "unexpected internal error");
    g_MonoServer->Restart();
}
