/*
 * =====================================================================================
 *
 *       Filename: monsterop.cpp
 *        Created: 05/03/2016 21:49:38
 *    Description:
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

#include <algorithm>
#include "player.hpp"
#include "monster.hpp"
#include "sysconst.hpp"
#include "actorpod.hpp"
#include "mathf.hpp"
#include "monoserver.hpp"
#include "dbcomid.hpp"

extern MonoServer *g_monoServer;
void Monster::on_AM_METRONOME(const ActorMsgPack &)
{
    update();
}

void Monster::on_AM_MISS(const ActorMsgPack &mpk)
{
    const auto amM = mpk.conv<AMMiss>();
    if(amM.UID != UID()){
        return;
    }

    SMMiss smM;
    std::memset(&smM, 0, sizeof(smM));

    smM.UID = amM.UID;
    dispatchInViewCONetPackage(SM_MISS, smM);
}

void Monster::on_AM_HEAL(const ActorMsgPack &mpk)
{
    const auto amH = mpk.conv<AMHeal>();
    if(amH.mapID == mapID()){
        m_sdHealth.HP += amH.addHP;
        m_sdHealth.MP += amH.addMP;
        dispatchHealth();
    }
}

void Monster::on_AM_QUERYCORECORD(const ActorMsgPack &rstMPK)
{
    AMQueryCORecord amQCOR;
    std::memcpy(&amQCOR, rstMPK.data(), sizeof(amQCOR));

    reportCO(amQCOR.UID);
}

void Monster::on_AM_EXP(const ActorMsgPack &rstMPK)
{
    if(masterUID()){
        m_actorPod->forward(masterUID(), {rstMPK.type(), rstMPK.data(), rstMPK.size()});
    }
}

void Monster::on_AM_ACTION(const ActorMsgPack &rstMPK)
{
    const auto amA = rstMPK.conv<AMAction>();
    if(amA.UID == UID()){
        return;
    }

    const auto addedInView = updateInViewCO(COLocation
    {
        .uid = amA.UID,
        .mapID = amA.mapID,

        .x = amA.action.x,
        .y = amA.action.y,
        .direction = amA.action.direction,
    });

    if(addedInView){
        switch(amA.action.type){
            case ACTION_SPAWN:
            case ACTION_SPACEMOVE2:
                {
                    dispatchAction(amA.UID, makeActionStand());
                    break;
                }
            default:
                {
                    break;
                }
        }

        if(uidf::isPlayer(amA.UID)){
            dispatchHealth(); // TODO only dispatch to the added CO
            m_actorPod->setMetronomeFreq(10);
        }
    }
}

void Monster::on_AM_NOTIFYNEWCO(const ActorMsgPack &rstMPK)
{
    const auto amNNCO = rstMPK.conv<AMNotifyNewCO>();
    if(m_dead.get()){
        notifyDead(amNNCO.UID);
    }
    else{
        // should make an valid action node and send it
        // currently just dispatch through map
        dispatchAction(makeActionStand());
    }
}

void Monster::on_AM_ATTACK(const ActorMsgPack &mpk)
{
    onAMAttack(mpk);
}

void Monster::on_AM_MAPSWITCH(const ActorMsgPack &)
{
}

void Monster::on_AM_QUERYLOCATION(const ActorMsgPack &rstMPK)
{
    AMLocation amL;
    std::memset(&amL, 0, sizeof(amL));

    amL.UID       = UID();
    amL.mapID     = mapID();
    amL.X         = X();
    amL.Y         = Y();
    amL.Direction = Direction();

    m_actorPod->forward(rstMPK.from(), {AM_LOCATION, amL}, rstMPK.seqID());
}

void Monster::on_AM_UPDATEHP(const ActorMsgPack &)
{
}

void Monster::on_AM_BADACTORPOD(const ActorMsgPack &)
{
}

void Monster::on_AM_DEADFADEOUT(const ActorMsgPack &mpk)
{
    const auto amDFO = mpk.conv<AMDeadFadeOut>();
    m_inViewCOList.erase(amDFO.UID);
}

void Monster::on_AM_NOTIFYDEAD(const ActorMsgPack &rstMPK)
{
    const auto amND = rstMPK.conv<AMNotifyDead>();
    m_inViewCOList.erase(amND.UID);
}

void Monster::on_AM_OFFLINE(const ActorMsgPack &rstMPK)
{
    AMOffline amO;
    std::memcpy(&amO, rstMPK.data(), sizeof(amO));

    if(true
            && amO.UID
            && amO.UID == masterUID()){
        goDie();
    }
}

void Monster::on_AM_CHECKMASTER(const ActorMsgPack &rstMPK)
{
    m_actorPod->forward(rstMPK.from(), AM_OK, rstMPK.seqID());
}

void Monster::on_AM_QUERYHEALTH(const ActorMsgPack &rmpk)
{
    m_actorPod->forward(rmpk.from(), {AM_HEALTH, cerealf::serialize(m_sdHealth)}, rmpk.seqID());
}

void Monster::on_AM_QUERYMASTER(const ActorMsgPack &mpk)
{
    AMUID amUID;
    std::memset(&amUID, 0, sizeof(amUID));

    amUID.UID = masterUID() ? masterUID() : UID();
    m_actorPod->forward(mpk.from(), {AM_UID, amUID}, mpk.seqID());
}

void Monster::on_AM_QUERYFINALMASTER(const ActorMsgPack &mpk)
{
    queryFinalMaster(UID(), [this, mpk](uint64_t finalMasterUID)
    {
        AMUID amUID;
        std::memset(&amUID, 0, sizeof(amUID));

        amUID.UID = finalMasterUID;
        m_actorPod->forward(mpk.from(), {AM_UID, amUID}, mpk.seqID());
    });
}

void Monster::on_AM_QUERYFRIENDTYPE(const ActorMsgPack &rstMPK)
{
    AMQueryFriendType amQFT;
    std::memcpy(&amQFT, rstMPK.data(), sizeof(amQFT));

    checkFriend(amQFT.UID, [this, rstMPK](int nFriendType)
    {
        AMFriendType amFT;
        std::memset(&amFT, 0, sizeof(amFT));

        amFT.Type = nFriendType;
        m_actorPod->forward(rstMPK.from(), {AM_FRIENDTYPE, amFT}, rstMPK.seqID());
    });
}

void Monster::on_AM_QUERYNAMECOLOR(const ActorMsgPack &rstMPK)
{
    AMNameColor amNC;
    std::memset(&amNC, 0, sizeof(amNC));

    amNC.Color = 'W';
    m_actorPod->forward(rstMPK.from(), {AM_NAMECOLOR, amNC}, rstMPK.seqID());
}

void Monster::on_AM_MASTERKILL(const ActorMsgPack &rstMPK)
{
    if(masterUID() && (rstMPK.from() == masterUID())){
        goDie();
    }
}

void Monster::on_AM_MASTERHITTED(const ActorMsgPack &mpk)
{
    onAMMasterHitted(mpk);
}
