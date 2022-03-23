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
#include "friendtype.hpp"
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
        updateHealth(amH.addHP, amH.addMP);
    }
}

void Monster::on_AM_QUERYCORECORD(const ActorMsgPack &rstMPK)
{
    AMQueryCORecord amQCOR;
    std::memcpy(&amQCOR, rstMPK.data(), sizeof(amQCOR));

    reportCO(amQCOR.UID);
}

void Monster::on_AM_QUERYUIDBUFF(const ActorMsgPack &mpk)
{
    forwardNetPackage(mpk.from(), SM_BUFFIDLIST, cerealf::serialize(SDBuffIDList
    {
        .uid = UID(),
        .idList = m_buffList.getIDList(),
    }));
}

void Monster::on_AM_ADDBUFF(const ActorMsgPack &mpk)
{
    const auto amAB = mpk.conv<AMAddBuff>();
    fflassert(amAB.id);
    fflassert(DBCOM_BUFFRECORD(amAB.id));

    checkFriend(amAB.fromUID, [amAB, this](int friendType)
    {
        const auto &br = DBCOM_BUFFRECORD(amAB.id);
        fflassert(br);

        switch(friendType){
            case FT_FRIEND:
                {
                    if(br.favor >= 0){
                        addBuff(amAB.fromUID, amAB.fromBuffSeq, amAB.id);
                    }
                    return;
                }
            case FT_ENEMY:
                {
                    if(br.favor <= 0){
                        addBuff(amAB.fromUID, amAB.fromBuffSeq, amAB.id);
                    }
                    return;
                }
            case FT_NEUTRAL:
                {
                    addBuff(amAB.fromUID, amAB.fromBuffSeq, amAB.id);
                    return;
                }
            default:
                {
                    return;
                }
        }
    });
}

void Monster::on_AM_REMOVEBUFF(const ActorMsgPack &mpk)
{
    const auto amRB = mpk.conv<AMRemoveBuff>();
    removeFromBuff(amRB.fromUID, amRB.fromBuffSeq, true);
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

    const auto [dstX, dstY] = [&amA]() -> std::tuple<int, int>
    {
        switch(amA.action.type){
            case ACTION_MOVE:
            case ACTION_SPACEMOVE:
                {
                    return {amA.action.aimX, amA.action.aimY};
                }
            default:
                {
                    return {amA.action.x, amA.action.y};
                }
        }
    }();

    const auto distChanged = [dstX, dstY, amA, this]() -> bool
    {
        if(amA.mapID != mapID()){
            return true;
        }

        if(const auto coLocPtr = getInViewCOPtr(amA.UID)){
            return mathf::LDistance2<int>(X(), Y(), coLocPtr->x, coLocPtr->y) != mathf::LDistance2<int>(X(), Y(), dstX, dstY);
        }
        return true;
    }();

    const auto addedInView = updateInViewCO(COLocation
    {
        .uid = amA.UID,
        .mapID = amA.mapID,

        .x = dstX,
        .y = dstY,
        .direction = amA.action.direction,
    });

    if(distChanged){
        m_buffList.updateAura(amA.UID);
    }

    // if sent is a player and is removed from this inview CO list
    // then this CO doesn't need to send its location to player, player should call trimInViewCO()

    if(addedInView > 0){
        dispatchAction(amA.UID, makeActionStand());
        if(uidf::isPlayer(amA.UID)){
            dispatchHealth(amA.UID);
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

void Monster::on_AM_MAPSWITCHTRIGGER(const ActorMsgPack &)
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
