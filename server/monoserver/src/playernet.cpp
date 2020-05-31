/*
 * =====================================================================================
 *
 *       Filename: playernet.cpp
 *        Created: 05/19/2016 15:26:25
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

        AMQueryCORecord stAMQCOR;
        std::memset(&stAMQCOR, 0, sizeof(stAMQCOR));

        // target UID can ignore it
        // send the query without response requirement

        stAMQCOR.UID = UID();
        if(!m_actorPod->forward(stCMQCOR.AimUID, {MPK_QUERYCORECORD, stAMQCOR})){
            ReportDeadUID(stCMQCOR.AimUID);
        }
    }
}

void Player::Net_CM_REQUESTSPACEMOVE(uint8_t, const uint8_t *pBuf, size_t)
{
    CMReqestSpaceMove stCMRSM;
    std::memcpy(&stCMRSM, pBuf, sizeof(stCMRSM));
    requestSpaceMove(stCMRSM.MapID, stCMRSM.X, stCMRSM.Y, false, [](){}, [](){});
}

void Player::Net_CM_REQUESTKILLPETS(uint8_t, const uint8_t *, size_t)
{
    RequestKillPets();
}

void Player::Net_CM_PICKUP(uint8_t, const uint8_t *pBuf, size_t)
{
    if(auto pCM = (CMPickUp *)(pBuf); pCM->MapID == m_map->ID()){
        if(CanPickUp(pCM->ID, 0)){
            AMPickUp stAMPU;
            std::memset(&stAMPU, 0, sizeof(stAMPU));
            stAMPU.X    = pCM->X;
            stAMPU.Y    = pCM->Y;
            stAMPU.UID  = pCM->UID;
            stAMPU.ID   = pCM->ID;
            stAMPU.DBID = pCM->DBID;
            m_actorPod->forward(m_map->UID(), {MPK_PICKUP, stAMPU});
        }
    }
}

void Player::Net_CM_QUERYGOLD(uint8_t, const uint8_t *, size_t)
{
    ReportGold();
}

void Player::Net_CM_NPCEVENT(uint8_t, const uint8_t *buf, size_t bufLen)
{
    const auto cmNPCE = ClientMsg::conv<CMNPCEvent>(buf, bufLen);
    AMNPCEvent amNPCEvent;

    std::memset(&amNPCEvent, 0, sizeof(amNPCEvent));
    amNPCEvent.x = X();
    amNPCEvent.y = Y();
    amNPCEvent.mapID = MapID();
    std::strcpy(amNPCEvent.event, cmNPCE.event);
    std::strcpy(amNPCEvent.value, cmNPCE.value);
    m_actorPod->forward(cmNPCE.uid, {MPK_NPCEVENT, amNPCEvent});
}
