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
void Monster::on_MPK_METRONOME(const MessagePack &)
{
    update();
}

void Monster::on_MPK_MISS(const MessagePack &rstMPK)
{
    AMMiss amM;
    std::memcpy(&amM, rstMPK.Data(), sizeof(amM));

    if(amM.UID != UID()){
        return;
    }

    foreachInViewCO([this, amM](const COLocation &rstLocation)
    {
        if(uidf::getUIDType(rstLocation.UID) == UID_PLY){
            m_actorPod->forward(rstLocation.UID, {MPK_MISS, amM});
        }
    });
}

void Monster::on_MPK_QUERYCORECORD(const MessagePack &rstMPK)
{
    AMQueryCORecord amQCOR;
    std::memcpy(&amQCOR, rstMPK.Data(), sizeof(amQCOR));

    reportCO(amQCOR.UID);
}

void Monster::on_MPK_EXP(const MessagePack &rstMPK)
{
    if(masterUID()){
        m_actorPod->forward(masterUID(), {rstMPK.Type(), rstMPK.Data(), rstMPK.DataLen()});
    }
}

void Monster::on_MPK_ACTION(const MessagePack &rstMPK)
{
    AMAction amA;
    std::memcpy(&amA, rstMPK.Data(), sizeof(amA));

    if(amA.UID == UID()){
        return;
    }

    if(amA.MapID != MapID()){
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
                return;
            }
    }

    switch(amA.action.type){
        case ACTION_SPAWN:
        case ACTION_SPACEMOVE2:
            {
                dispatchAction(amA.UID, _ActionStand
                {
                    .x = X(),
                    .y = Y(),
                    .direction = Direction(),
                });
                break;
            }
        default:
            {
                break;
            }
    }

    AddInViewCO(amA.UID, amA.MapID, nX, nY, nDir);
}

void Monster::on_MPK_NOTIFYNEWCO(const MessagePack &rstMPK)
{
    const auto amNNCO = rstMPK.conv<AMNotifyNewCO>();
    if(m_dead.get()){
        notifyDead(amNNCO.UID);
    }
    else{
        // should make an valid action node and send it
        // currently just dispatch through map
        dispatchAction(_ActionStand
        {
            .x = X(),
            .y = Y(),
            .direction = Direction(),
        });
    }
}

void Monster::on_MPK_ATTACK(const MessagePack &rstMPK)
{
    AMAttack amAK;
    std::memcpy(&amAK, rstMPK.Data(), sizeof(amAK));

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
                        m_actorPod->forward(amAK.UID, {MPK_MISS, amM});
                        return;
                    }
                default:
                    {
                        return;
                    }
            }
        }

        addOffenderDamage(amAK.UID, amAK.Damage);
        dispatchAction(_ActionHitted
        {
            .x = X(),
            .y = Y(),
            .direction = Direction(),
        });
        StruckDamage({amAK.UID, amAK.Type, amAK.Damage, amAK.Element, amAK.Effect});
    }
}

void Monster::on_MPK_MAPSWITCH(const MessagePack &)
{
}

void Monster::on_MPK_QUERYLOCATION(const MessagePack &rstMPK)
{
    AMLocation amL;
    std::memset(&amL, 0, sizeof(amL));

    amL.UID       = UID();
    amL.MapID     = MapID();
    amL.X         = X();
    amL.Y         = Y();
    amL.Direction = Direction();

    m_actorPod->forward(rstMPK.from(), {MPK_LOCATION, amL}, rstMPK.ID());
}

void Monster::on_MPK_UPDATEHP(const MessagePack &)
{
}

void Monster::on_MPK_BADACTORPOD(const MessagePack &)
{
}

void Monster::on_MPK_DEADFADEOUT(const MessagePack &rstMPK)
{
    AMDeadFadeOut amDFO;
    std::memcpy(&amDFO, rstMPK.Data(), sizeof(amDFO));

    removeTarget(amDFO.UID);
    RemoveInViewCO(amDFO.UID);
}

void Monster::on_MPK_NOTIFYDEAD(const MessagePack &rstMPK)
{
    AMNotifyDead amND;
    std::memcpy(&amND, rstMPK.Data(), sizeof(amND));

    removeTarget(amND.UID);
    RemoveInViewCO(amND.UID);
}

void Monster::on_MPK_OFFLINE(const MessagePack &rstMPK)
{
    AMOffline amO;
    std::memcpy(&amO, rstMPK.Data(), sizeof(amO));

    if(true
            && amO.UID
            && amO.UID == masterUID()){
        goDie();
    }
}

void Monster::on_MPK_CHECKMASTER(const MessagePack &rstMPK)
{
    m_actorPod->forward(rstMPK.from(), MPK_OK, rstMPK.ID());
}

void Monster::on_MPK_QUERYMASTER(const MessagePack &rstMPK)
{
    AMUID amUID;
    std::memset(&amUID, 0, sizeof(amUID));

    amUID.UID = masterUID() ? masterUID() : UID();
    m_actorPod->forward(rstMPK.from(), {MPK_UID, amUID}, rstMPK.ID());
}

void Monster::on_MPK_QUERYFINALMASTER(const MessagePack &rstMPK)
{
    QueryFinalMaster(UID(), [this, rstMPK](uint64_t nFMasterUID)
    {
        AMUID amUID;
        std::memset(&amUID, 0, sizeof(amUID));

        amUID.UID = nFMasterUID;
        m_actorPod->forward(rstMPK.from(), {MPK_UID, amUID}, rstMPK.ID());
    });
}

void Monster::on_MPK_QUERYFRIENDTYPE(const MessagePack &rstMPK)
{
    AMQueryFriendType amQFT;
    std::memcpy(&amQFT, rstMPK.Data(), sizeof(amQFT));

    checkFriend(amQFT.UID, [this, rstMPK](int nFriendType)
    {
        AMFriendType amFT;
        std::memset(&amFT, 0, sizeof(amFT));

        amFT.Type = nFriendType;
        m_actorPod->forward(rstMPK.from(), {MPK_FRIENDTYPE, amFT}, rstMPK.ID());
    });
}

void Monster::on_MPK_QUERYNAMECOLOR(const MessagePack &rstMPK)
{
    AMNameColor amNC;
    std::memset(&amNC, 0, sizeof(amNC));

    amNC.Color = 'W';
    m_actorPod->forward(rstMPK.from(), {MPK_NAMECOLOR, amNC}, rstMPK.ID());
}

void Monster::on_MPK_MASTERKILL(const MessagePack &rstMPK)
{
    if(masterUID() && (rstMPK.from() == masterUID())){
        goDie();
    }
}

void Monster::on_MPK_MASTERHITTED(const MessagePack &rstMPK)
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
