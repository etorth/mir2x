/*
 * =====================================================================================
 *
 *       Filename: playernet.cpp
 *        Created: 05/19/2016 15:26:25
 *  Last Modified: 01/25/2018 21:17:23
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
            && stCMA.UID == UID()
            && stCMA.MapID == MapID()){

        switch((int)(stCMA.Action)){
            case ACTION_STAND : OnCMActionStand (stCMA); return;
            case ACTION_MOVE  : OnCMActionMove  (stCMA); return;
            case ACTION_ATTACK: OnCMActionAttack(stCMA); return;
            case ACTION_SPELL : OnCMActionSpell (stCMA); return;
            case ACTION_PICKUP: OnCMActionPickUp(stCMA); return;
            default           :                          return;
        }
    }
}

void Player::Net_CM_QUERYCORECORD(uint8_t, const uint8_t *pBuf, size_t)
{
    CMQueryCORecord stCMQCOR;
    std::memcpy(&stCMQCOR, pBuf, sizeof(stCMQCOR));

    if(true
            && stCMQCOR.AimUID
            && stCMQCOR.AimUID != UID()){

        extern MonoServer *g_MonoServer;
        if(auto stUIDRecord = g_MonoServer->GetUIDRecord(stCMQCOR.AimUID)){

            AMQueryCORecord stAMQCOR;
            std::memset(&stAMQCOR, 0, sizeof(stAMQCOR));

            // target UID can ignore it
            // send the query without response requirement

            stAMQCOR.UID = UID();
            m_ActorPod->Forward({MPK_QUERYCORECORD, stAMQCOR}, stUIDRecord.GetAddress());
        }
    }
}

void Player::Net_CM_REQUESTSPACEMOVE(uint8_t, const uint8_t *pBuf, size_t)
{
    CMReqestSpaceMove stCMRSM;
    std::memcpy(&stCMRSM, pBuf, sizeof(stCMRSM));
    RequestSpaceMove(stCMRSM.MapID, stCMRSM.X, stCMRSM.Y, false, [](){}, [](){});
}

void Player::Net_CM_PICKUP(uint8_t, const uint8_t *pBuf, size_t)
{
    auto pCM = (CMPickUp *)(pBuf);
    if(pCM->MapID == m_Map->ID()){
        if(CanPickUp(pCM->ID, 0)){
            AMPickUp stAMPU;
            std::memset(&stAMPU, 0, sizeof(stAMPU));
            stAMPU.X    = pCM->X;
            stAMPU.Y    = pCM->Y;
            stAMPU.UID  = pCM->UID;
            stAMPU.ID   = pCM->ID;
            stAMPU.DBID = pCM->DBID;
            m_ActorPod->Forward({MPK_PICKUP, stAMPU}, m_Map->GetAddress());
        }
    }
}

void Player::Net_CM_QUERYGOLD(uint8_t, const uint8_t *, size_t)
{
    ReportGold();
}
