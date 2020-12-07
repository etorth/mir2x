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
        RemoveTarget(amA.UID);
        RemoveInViewCO(amA.UID);
        return;
    }

    int nX   = -1;
    int nY   = -1;
    int nDir = -1;

    switch(amA.Action){
        case ACTION_STAND:
        case ACTION_ATTACK:
        case ACTION_HITTED:
            {
                nX   = amA.X;
                nY   = amA.Y;
                nDir = amA.Direction;
                break;
            }
        case ACTION_MOVE:
        case ACTION_SPELL:
        case ACTION_SPAWN:
            {
                nX = amA.X;
                nY = amA.Y;
                break;
            }
        case ACTION_DIE:
            {
                RemoveTarget(amA.UID);
                RemoveInViewCO(amA.UID);
                return;
            }
        default:
            {
                return;
            }
    }

    switch(amA.Action){
        case ACTION_SPAWN:
        case ACTION_SPACEMOVE2:
            {
                dispatchAction(amA.UID, ActionStand(X(), Y(), Direction()));
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
    switch(GetState(STATE_DEAD)){
        case 0:
            {
                // should make an valid action node and send it
                // currently just dispatch through map

                dispatchAction(ActionStand(X(), Y(), Direction()));
                break;
            }
        default:
            {
                AMNotifyDead amND;
                std::memset(&amND, 0, sizeof(amND));

                amND.UID = UID();
                m_actorPod->forward(amNNCO.UID, {MPK_NOTIFYDEAD, amND});
                break;
            }
    }
}

void Monster::on_MPK_ATTACK(const MessagePack &rstMPK)
{
    AMAttack amAK;
    std::memcpy(&amAK, rstMPK.Data(), sizeof(amAK));

    switch(GetState(STATE_DEAD)){
        case 0:
            {
                if(monsterID() == DBCOM_MONSTERID(u8"神兽")){
                    dynamic_cast<TaoDog *>(this)->setTransf(true);
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
                dispatchAction(ActionHitted(X(), Y(), Direction()));
                StruckDamage({amAK.UID, amAK.Type, amAK.Damage, amAK.Element, amAK.Effect});
                return;
            }
        default:
            {
                AMNotifyDead amND;
                std::memset(&amND, 0, sizeof(amND));

                amND.UID = UID();
                m_actorPod->forward(amAK.UID, {MPK_NOTIFYDEAD, amND});
                return;
            }
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

    RemoveTarget(amDFO.UID);
    RemoveInViewCO(amDFO.UID);
}

void Monster::on_MPK_NOTIFYDEAD(const MessagePack &rstMPK)
{
    AMNotifyDead amND;
    std::memcpy(&amND, rstMPK.Data(), sizeof(amND));

    RemoveTarget(amND.UID);
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
        if(checkMonsterName(u8"神兽")){
            dynamic_cast<TaoDog *>(this)->setTransf(true);
            return;
        }

        if(checkMonsterName(u8"变异骷髅")){
            return;
        }
    }
}
