#include <algorithm>
#include "player.hpp"
#include "monster.hpp"
#include "sysconst.hpp"
#include "actorpod.hpp"
#include "mathf.hpp"
#include "server.hpp"
#include "friendtype.hpp"
#include "dbcomid.hpp"

extern Server *g_server;
corof::entrance Monster::on_AM_METRONOME(const ActorMsgPack &)
{
    update();
}

corof::entrance Monster::on_AM_MISS(const ActorMsgPack &mpk)
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

corof::entrance Monster::on_AM_HEAL(const ActorMsgPack &mpk)
{
    const auto amH = mpk.conv<AMHeal>();
    if(amH.mapUID == mapUID()){
        updateHealth(amH.addHP, amH.addMP);
    }
}

corof::entrance Monster::on_AM_QUERYCORECORD(const ActorMsgPack &rstMPK)
{
    AMQueryCORecord amQCOR;
    std::memcpy(&amQCOR, rstMPK.data(), sizeof(amQCOR));

    reportCO(amQCOR.UID);
}

corof::entrance Monster::on_AM_QUERYUIDBUFF(const ActorMsgPack &mpk)
{
    forwardNetPackage(mpk.from(), SM_BUFFIDLIST, cerealf::serialize(SDBuffIDList
    {
        .uid = UID(),
        .idList = m_buffList.getIDList(),
    }));
}

corof::entrance Monster::on_AM_ADDBUFF(const ActorMsgPack &mpk)
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

corof::entrance Monster::on_AM_REMOVEBUFF(const ActorMsgPack &mpk)
{
    const auto amRB = mpk.conv<AMRemoveBuff>();
    removeFromBuff(amRB.fromUID, amRB.fromBuffSeq, true);
}

corof::entrance Monster::on_AM_EXP(const ActorMsgPack &rstMPK)
{
    if(masterUID()){
        m_actorPod->post(masterUID(), {rstMPK.type(), rstMPK.data(), rstMPK.size()});
    }
}

corof::entrance Monster::on_AM_ACTION(const ActorMsgPack &rstMPK)
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
        if(amA.mapUID != mapUID()){
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
        .mapUID = amA.mapUID,

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

corof::entrance Monster::on_AM_NOTIFYNEWCO(const ActorMsgPack &rstMPK)
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

corof::entrance Monster::on_AM_ATTACK(const ActorMsgPack &mpk)
{
    onAMAttack(mpk);
}

corof::entrance Monster::on_AM_MAPSWITCHTRIGGER(const ActorMsgPack &)
{
}

corof::entrance Monster::on_AM_QUERYLOCATION(const ActorMsgPack &rstMPK)
{
    AMLocation amL;
    std::memset(&amL, 0, sizeof(amL));

    amL.UID       = UID();
    amL.mapUID    = mapUID();
    amL.X         = X();
    amL.Y         = Y();
    amL.Direction = Direction();

    m_actorPod->post(rstMPK.fromAddr(), {AM_LOCATION, amL});
}

corof::entrance Monster::on_AM_UPDATEHP(const ActorMsgPack &)
{
}

corof::entrance Monster::on_AM_BADACTORPOD(const ActorMsgPack &)
{
}

corof::entrance Monster::on_AM_DEADFADEOUT(const ActorMsgPack &mpk)
{
    const auto amDFO = mpk.conv<AMDeadFadeOut>();
    m_inViewCOList.erase(amDFO.UID);
}

corof::entrance Monster::on_AM_NOTIFYDEAD(const ActorMsgPack &rstMPK)
{
    const auto amND = rstMPK.conv<AMNotifyDead>();
    m_inViewCOList.erase(amND.UID);
}

corof::entrance Monster::on_AM_OFFLINE(const ActorMsgPack &rstMPK)
{
    AMOffline amO;
    std::memcpy(&amO, rstMPK.data(), sizeof(amO));

    if(true
            && amO.UID
            && amO.UID == masterUID()){
        goDie();
    }
}

corof::entrance Monster::on_AM_CHECKMASTER(const ActorMsgPack &rstMPK)
{
    m_actorPod->post(rstMPK.fromAddr(), AM_OK);
}

corof::entrance Monster::on_AM_QUERYHEALTH(const ActorMsgPack &rmpk)
{
    m_actorPod->post(rmpk.fromAddr(), {AM_HEALTH, cerealf::serialize(m_sdHealth)});
}

corof::entrance Monster::on_AM_QUERYMASTER(const ActorMsgPack &mpk)
{
    AMUID amUID;
    std::memset(&amUID, 0, sizeof(amUID));

    amUID.uid = masterUID() ? masterUID() : UID();
    m_actorPod->post(mpk.fromAddr(), {AM_UID, amUID});
}

corof::entrance Monster::on_AM_QUERYFINALMASTER(const ActorMsgPack &mpk)
{
    queryFinalMaster(UID(), [this, mpk](uint64_t finalMasterUID)
    {
        AMUID amUID;
        std::memset(&amUID, 0, sizeof(amUID));

        amUID.uid = finalMasterUID;
        m_actorPod->post(mpk.fromAddr(), {AM_UID, amUID});
    });
}

corof::entrance Monster::on_AM_QUERYFRIENDTYPE(const ActorMsgPack &rstMPK)
{
    AMQueryFriendType amQFT;
    std::memcpy(&amQFT, rstMPK.data(), sizeof(amQFT));

    checkFriend(amQFT.UID, [this, rstMPK](int nFriendType)
    {
        AMFriendType amFT;
        std::memset(&amFT, 0, sizeof(amFT));

        amFT.Type = nFriendType;
        m_actorPod->post(rstMPK.fromAddr(), {AM_FRIENDTYPE, amFT});
    });
}

corof::entrance Monster::on_AM_QUERYNAMECOLOR(const ActorMsgPack &rstMPK)
{
    AMNameColor amNC;
    std::memset(&amNC, 0, sizeof(amNC));

    amNC.Color = 'W';
    m_actorPod->post(rstMPK.fromAddr(), {AM_NAMECOLOR, amNC});
}

corof::entrance Monster::on_AM_MASTERKILL(const ActorMsgPack &rstMPK)
{
    if(masterUID() && (rstMPK.from() == masterUID())){
        goDie();
    }
}

corof::entrance Monster::on_AM_MASTERHITTED(const ActorMsgPack &mpk)
{
    onAMMasterHitted(mpk);
}
