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

void Player::net_CM_ACTION(uint8_t, const uint8_t *pBuf, size_t)
{
    CMAction cmA;
    std::memcpy(&cmA, pBuf, sizeof(cmA));

    if(true
            && cmA.UID == UID()
            && cmA.MapID == MapID()){

        switch((int)(cmA.action.type)){
            case ACTION_STAND : onCMActionStand (cmA); return;
            case ACTION_MOVE  : onCMActionMove  (cmA); return;
            case ACTION_ATTACK: onCMActionAttack(cmA); return;
            case ACTION_SPELL : onCMActionSpell (cmA); return;
            case ACTION_PICKUP: onCMActionPickUp(cmA); return;
            default           :                        return;
        }
        dispatchAction(cmA.action);
    }
}

void Player::net_CM_QUERYCORECORD(uint8_t, const uint8_t *pBuf, size_t)
{
    CMQueryCORecord stCMQCOR;
    std::memcpy(&stCMQCOR, pBuf, sizeof(stCMQCOR));

    if(true
            && stCMQCOR.AimUID
            && stCMQCOR.AimUID != UID()){

        AMQueryCORecord amQCOR;
        std::memset(&amQCOR, 0, sizeof(amQCOR));

        // target UID can ignore it
        // send the query without response requirement

        amQCOR.UID = UID();
        if(!m_actorPod->forward(stCMQCOR.AimUID, {MPK_QUERYCORECORD, amQCOR})){
            reportDeadUID(stCMQCOR.AimUID);
        }
    }
}

void Player::net_CM_REQUESTSPACEMOVE(uint8_t, const uint8_t *buf, size_t)
{
    const auto cmRSM = ClientMsg::conv<CMRequestSpaceMove>(buf);
    if(cmRSM.MapID == MapID()){
        requestSpaceMove(cmRSM.X, cmRSM.Y, false);
    }
    else{
        requestMapSwitch(cmRSM.MapID, cmRSM.X, cmRSM.Y, false);
    }
}

void Player::net_CM_REQUESTMAGICDAMAGE(uint8_t, const uint8_t *buf, size_t)
{
    const auto cmRMD = ClientMsg::conv<CMRequestMagicDamage>(buf);
    dispatchAttack(cmRMD.aimUID, DC_PHY_PLAIN);
}

void Player::net_CM_REQUESTKILLPETS(uint8_t, const uint8_t *, size_t)
{
    RequestKillPets();
}

void Player::net_CM_PICKUP(uint8_t, const uint8_t *pBuf, size_t)
{
    if(auto pCM = (CMPickUp *)(pBuf); pCM->MapID == m_map->ID()){
        if(CanPickUp(pCM->ID, 0)){
            AMPickUp amPU;
            std::memset(&amPU, 0, sizeof(amPU));
            amPU.X    = pCM->X;
            amPU.Y    = pCM->Y;
            amPU.UID  = pCM->UID;
            amPU.ID   = pCM->ID;
            amPU.DBID = pCM->DBID;
            m_actorPod->forward(m_map->UID(), {MPK_PICKUP, amPU});
        }
    }
}

void Player::net_CM_PING(uint8_t, const uint8_t *pBuf, size_t)
{
    SMPing smP;
    std::memset(&smP, 0, sizeof(smP));
    smP.Tick = ((CMPing *)(pBuf))->Tick; // strict-aliasing issue
    postNetMessage(SM_PING, smP);
}

void Player::net_CM_QUERYGOLD(uint8_t, const uint8_t *, size_t)
{
    reportGold();
}

void Player::net_CM_NPCEVENT(uint8_t, const uint8_t *buf, size_t bufLen)
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

void Player::net_CM_QUERYSELLITEM(uint8_t, const uint8_t *buf, size_t bufLen)
{
    const auto cmQSI = ClientMsg::conv<CMQuerySellItem>(buf, bufLen);
    AMQuerySellItem amQSI;

    std::memset(&amQSI, 0, sizeof(amQSI));
    amQSI.itemID = cmQSI.itemID;
    m_actorPod->forward(cmQSI.npcUID, {MPK_QUERYSELLITEM, amQSI});
}

void Player::net_CM_QUERYPLAYERLOOK(uint8_t, const uint8_t *buf, size_t bufLen)
{
    const auto cmQPL = ClientMsg::conv<CMQueryPlayerLook>(buf, bufLen);
    if(cmQPL.uid == UID()){
        SMPlayerLook smPL;
        std::memset(&smPL, 0, sizeof(smPL));

        smPL.uid = UID();
        smPL.look = getPlayerLook();
        postNetMessage(SM_PLAYERLOOK, smPL);
    }
    else if(uidf::getUIDType(cmQPL.uid) == UID_PLY){
        m_actorPod->forward(cmQPL.uid, {MPK_QUERYPLAYERLOOK});
    }
    else{
        throw fflerror("invalid uid: %llu, type: %s", to_llu(cmQPL.uid), uidf::getUIDTypeString(cmQPL.uid));
    }
}

void Player::net_CM_QUERYPLAYERWEAR(uint8_t, const uint8_t *buf, size_t bufLen)
{
    const auto cmQPW = ClientMsg::conv<CMQueryPlayerWear>(buf, bufLen);
    if(cmQPW.uid == UID()){
        SMPlayerWear smPW;
        std::memset(&smPW, 0, sizeof(smPW));

        smPW.uid = UID();
        smPW.wear = getPlayerWear();
        postNetMessage(SM_PLAYERWEAR, smPW);
    }
    else if(uidf::getUIDType(cmQPW.uid) == UID_PLY){
        m_actorPod->forward(cmQPW.uid, {MPK_QUERYPLAYERWEAR});
    }
    else{
        throw fflerror("invalid uid: %llu, type: %s", to_llu(cmQPW.uid), uidf::getUIDTypeString(cmQPW.uid));
    }
}
