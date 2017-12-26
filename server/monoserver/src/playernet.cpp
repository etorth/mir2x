/*
 * =====================================================================================
 *
 *       Filename: playernet.cpp
 *        Created: 05/19/2016 15:26:25
 *  Last Modified: 12/25/2017 17:31:59
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
            && stCMQCOR.UID
            && stCMQCOR.MapID == MapID()

            && m_Map
            && m_Map->ValidC(stCMQCOR.X, stCMQCOR.Y)
            && m_Map->ActorPodValid()){
        // 1. check cached actor address first
        // 2. then send to map
        AMQueryCORecord stAMQCOR;
        std::memset(&stAMQCOR, 0, sizeof(stAMQCOR));

        stAMQCOR.UID    = UID();
        stAMQCOR.MapID  = stCMQCOR.MapID;
        stAMQCOR.X      = stCMQCOR.X;
        stAMQCOR.Y      = stCMQCOR.Y;
        stAMQCOR.AimUID = stCMQCOR.UID;
        m_ActorPod->Forward({MPK_QUERYCORECORD, stAMQCOR}, m_Map->GetAddress());
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
        AMPickUp stAMPU;
        stAMPU.X      = pCM->X;
        stAMPU.Y      = pCM->Y;
        stAMPU.UID    = pCM->UID;
        stAMPU.ItemID = pCM->ItemID;

        m_ActorPod->Forward({MPK_PICKUP, stAMPU}, m_Map->GetAddress());
    }
}
