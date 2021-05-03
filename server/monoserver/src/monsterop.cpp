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
#include "taodog.hpp"
#include "dbcomid.hpp"

extern MonoServer *g_monoServer;
void Monster::on_AM_METRONOME(const ActorMsgPack &)
{
    update();
}

void Monster::on_AM_MISS(const ActorMsgPack &rstMPK)
{
    AMMiss amM;
    std::memcpy(&amM, rstMPK.data(), sizeof(amM));

    if(amM.UID != UID()){
        return;
    }

    foreachInViewCO([this, amM](const COLocation &rstLocation)
    {
        if(uidf::getUIDType(rstLocation.uid) == UID_PLY){
            m_actorPod->forward(rstLocation.uid, {AM_MISS, amM});
        }
    });
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
    AMAction amA;
    std::memcpy(&amA, rstMPK.data(), sizeof(amA));

    if(amA.UID == UID()){
        return;
    }

    if(amA.mapID != mapID()){
        removeTarget(amA.UID);
        RemoveInViewCO(amA.UID);
        return;
    }

    int nX   = -1;
    int nY   = -1;
    int nDir = -1;

    switch(amA.action.type){
        case ACTION_STAND:
        case ACTION_ATTACK:
        case ACTION_HITTED:
            {
                nX   = amA.action.x;
                nY   = amA.action.y;
                nDir = amA.action.direction;
                break;
            }
        case ACTION_MOVE:
        case ACTION_SPELL:
        case ACTION_SPAWN:
            {
                nX = amA.action.x;
                nY = amA.action.y;
                break;
            }
        case ACTION_DIE:
            {
                removeTarget(amA.UID);
                RemoveInViewCO(amA.UID);
                return;
            }
        default:
            {
                break;
            }
    }

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

    AddInViewCO(amA.UID, amA.mapID, nX, nY, nDir);
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

void Monster::on_AM_ATTACK(const ActorMsgPack &rstMPK)
{
    AMAttack amAK;
    std::memcpy(&amAK, rstMPK.data(), sizeof(amAK));

    if(m_dead.get()){
        notifyDead(amAK.UID);
    }
    else{
        if(monsterName() == u8"神兽"){
            dynamic_cast<TaoDog *>(this)->setStandMode(true);
        }

        if(mathf::LDistance2(X(), Y(), amAK.X, amAK.Y) > 2){
            switch(uidf::getUIDType(amAK.UID)){
                case UID_MON:
                case UID_PLY:
                    {
                        AMMiss amM;
                        std::memset(&amM, 0, sizeof(amM));

                        amM.UID = amAK.UID;
                        m_actorPod->forward(amAK.UID, {AM_MISS, amM});
                        return;
                    }
                default:
                    {
                        return;
                    }
            }
        }

        addOffenderDamage(amAK.UID, amAK.Damage);
        dispatchAction(ActionHitted
        {
            .x = X(),
            .y = Y(),
            .direction = Direction(),
        });
        StruckDamage({amAK.UID, amAK.Type, amAK.Damage, amAK.Element, amAK.Effect});
    }
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

void Monster::on_AM_DEADFADEOUT(const ActorMsgPack &rstMPK)
{
    AMDeadFadeOut amDFO;
    std::memcpy(&amDFO, rstMPK.data(), sizeof(amDFO));

    removeTarget(amDFO.UID);
    RemoveInViewCO(amDFO.UID);
}

void Monster::on_AM_NOTIFYDEAD(const ActorMsgPack &rstMPK)
{
    AMNotifyDead amND;
    std::memcpy(&amND, rstMPK.data(), sizeof(amND));

    removeTarget(amND.UID);
    RemoveInViewCO(amND.UID);
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

void Monster::on_AM_QUERYMASTER(const ActorMsgPack &rstMPK)
{
    AMUID amUID;
    std::memset(&amUID, 0, sizeof(amUID));

    amUID.UID = masterUID() ? masterUID() : UID();
    m_actorPod->forward(rstMPK.from(), {AM_UID, amUID}, rstMPK.seqID());
}

void Monster::on_AM_QUERYFINALMASTER(const ActorMsgPack &rstMPK)
{
    QueryFinalMaster(UID(), [this, rstMPK](uint64_t nFMasterUID)
    {
        AMUID amUID;
        std::memset(&amUID, 0, sizeof(amUID));

        amUID.UID = nFMasterUID;
        m_actorPod->forward(rstMPK.from(), {AM_UID, amUID}, rstMPK.seqID());
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

void Monster::on_AM_MASTERHITTED(const ActorMsgPack &rstMPK)
{
    if(masterUID() && (rstMPK.from() == masterUID())){
        if(monsterName() == u8"神兽"){
            dynamic_cast<TaoDog *>(this)->setStandMode(true);
        }
        else if(monsterName() == u8"变异骷髅"){
            // ...
        }
        else{
            // ...
        }
    }
}
