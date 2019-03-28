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
#include "mathfunc.hpp"
#include "monoserver.hpp"

extern MonoServer *g_MonoServer;
void Monster::On_MPK_METRONOME(const MessagePack &)
{
    Update();
}

void Monster::On_MPK_QUERYCORECORD(const MessagePack &rstMPK)
{
    AMQueryCORecord stAMQCOR;
    std::memcpy(&stAMQCOR, rstMPK.Data(), sizeof(stAMQCOR));

    ReportCORecord(stAMQCOR.UID);
}

void Monster::On_MPK_EXP(const MessagePack &rstMPK)
{
    if(MasterUID()){
        m_ActorPod->Forward(MasterUID(), {rstMPK.Type(), rstMPK.Data(), rstMPK.DataLen()});
    }
}

void Monster::On_MPK_ACTION(const MessagePack &rstMPK)
{
    AMAction stAMA;
    std::memcpy(&stAMA, rstMPK.Data(), sizeof(stAMA));

    if(stAMA.UID == UID()){
        return;
    }

    if(stAMA.MapID != MapID()){
        RemoveTarget(stAMA.UID);
        RemoveInViewCO(stAMA.UID);
        return;
    }

    int nX   = -1;
    int nY   = -1;
    int nDir = -1;

    switch(stAMA.Action){
        case ACTION_STAND:
        case ACTION_ATTACK:
        case ACTION_HITTED:
            {
                nX   = stAMA.X;
                nY   = stAMA.Y;
                nDir = stAMA.Direction;
                break;
            }
        case ACTION_MOVE:
        case ACTION_SPELL:
        case ACTION_SPAWN:
            {
                nX = stAMA.X;
                nY = stAMA.Y;
                break;
            }
        case ACTION_DIE:
            {
                RemoveTarget(stAMA.UID);
                RemoveInViewCO(stAMA.UID);
                return;
            }
        default:
            {
                return;
            }
    }

    switch(stAMA.Action){
        case ACTION_SPAWN:
        case ACTION_SPACEMOVE2:
            {
                DispatchAction(stAMA.UID, ActionStand(X(), Y(), Direction()));
                break;
            }
        default:
            {
                break;
            }
    }

    AddInViewCO(stAMA.UID, stAMA.MapID, nX, nY, nDir);
}

void Monster::On_MPK_NOTIFYNEWCO(const MessagePack &rstMPK)
{
    AMNotifyNewCO stAMNNCO;
    std::memcpy(&stAMNNCO, rstMPK.Data(), sizeof(stAMNNCO));

    switch(GetState(STATE_DEAD)){
        case 0:
            {
                // should make an valid action node and send it
                // currently just dispatch through map

                DispatchAction(ActionStand(X(), Y(), Direction()));
                break;
            }
        default:
            {
                AMNotifyDead stAMND;
                std::memset(&stAMND, 0, sizeof(stAMND));

                stAMND.UID = UID();
                m_ActorPod->Forward(stAMNNCO.UID, {MPK_NOTIFYDEAD, stAMND});
                break;
            }
    }
}

void Monster::On_MPK_ATTACK(const MessagePack &rstMPK)
{
    AMAttack stAMAK;
    std::memcpy(&stAMAK, rstMPK.Data(), sizeof(stAMAK));

    switch(GetState(STATE_DEAD)){
        case 0:
            {
                if(MathFunc::LDistance2(X(), Y(), stAMAK.X, stAMAK.Y) >= 2){
                    return;
                }

                AddTarget(stAMAK.UID);
                DispatchAction(ActionHitted(X(), Y(), Direction()));

                AddOffenderDamage(stAMAK.UID, stAMAK.Damage);
                StruckDamage({stAMAK.UID, stAMAK.Type, stAMAK.Damage, stAMAK.Element, stAMAK.Effect});
                break;
            }
        default:
            {
                AMNotifyDead stAMND;
                std::memset(&stAMND, 0, sizeof(stAMND));

                stAMND.UID = UID();
                m_ActorPod->Forward(stAMAK.UID, {MPK_NOTIFYDEAD, stAMND});
                break;
            }
    }
}

void Monster::On_MPK_MAPSWITCH(const MessagePack &)
{
}

void Monster::On_MPK_QUERYLOCATION(const MessagePack &rstMPK)
{
    AMLocation stAML;
    std::memset(&stAML, 0, sizeof(stAML));

    stAML.UID       = UID();
    stAML.MapID     = MapID();
    stAML.X         = X();
    stAML.Y         = Y();
    stAML.Direction = Direction();

    m_ActorPod->Forward(rstMPK.From(), {MPK_LOCATION, stAML}, rstMPK.ID());
}

void Monster::On_MPK_UPDATEHP(const MessagePack &)
{
}

void Monster::On_MPK_BADACTORPOD(const MessagePack &)
{
}

void Monster::On_MPK_DEADFADEOUT(const MessagePack &rstMPK)
{
    AMDeadFadeOut stAMDFO;
    std::memcpy(&stAMDFO, rstMPK.Data(), sizeof(stAMDFO));

    RemoveTarget(stAMDFO.UID);
    RemoveInViewCO(stAMDFO.UID);
}

void Monster::On_MPK_NOTIFYDEAD(const MessagePack &rstMPK)
{
    AMNotifyDead stAMND;
    std::memcpy(&stAMND, rstMPK.Data(), sizeof(stAMND));

    RemoveTarget(stAMND.UID);
    RemoveInViewCO(stAMND.UID);
}

void Monster::On_MPK_OFFLINE(const MessagePack &rstMPK)
{
    AMOffline stAMO;
    std::memcpy(&stAMO, rstMPK.Data(), sizeof(stAMO));

    if(true
            && stAMO.UID
            && stAMO.UID == MasterUID()){
        GoDie();
    }
}

void Monster::On_MPK_CHECKMASTER(const MessagePack &rstMPK)
{
    m_ActorPod->Forward(rstMPK.From(), MPK_OK, rstMPK.ID());
}

void Monster::On_MPK_QUERYMASTER(const MessagePack &rstMPK)
{
    AMUID stAMUID;
    std::memset(&stAMUID, 0, sizeof(stAMUID));

    stAMUID.UID = MasterUID() ? MasterUID() : UID();
    m_ActorPod->Forward(rstMPK.From(), {MPK_UID, stAMUID}, rstMPK.ID());
}

void Monster::On_MPK_QUERYFINALMASTER(const MessagePack &rstMPK)
{
    QueryFinalMaster(UID(), [this, rstMPK](uint64_t nFMasterUID)
    {
        AMUID stAMUID;
        std::memset(&stAMUID, 0, sizeof(stAMUID));

        stAMUID.UID = nFMasterUID;
        m_ActorPod->Forward(rstMPK.From(), {MPK_UID, stAMUID}, rstMPK.ID());
    });
}

void Monster::On_MPK_QUERYFRIENDTYPE(const MessagePack &rstMPK)
{
    AMQueryFriendType stAMQFT;
    std::memcpy(&stAMQFT, rstMPK.Data(), sizeof(stAMQFT));

    CheckFriendType(stAMQFT.UID, [this, rstMPK](int nFriendType)
    {
        AMFriendType stAMFT;
        std::memset(&stAMFT, 0, sizeof(stAMFT));

        stAMFT.Type = nFriendType;
        m_ActorPod->Forward(rstMPK.From(), {MPK_FRIENDTYPE, stAMFT}, rstMPK.ID());
    });
}

void Monster::On_MPK_QUERYNAMECOLOR(const MessagePack &rstMPK)
{
    AMNameColor stAMNC;
    std::memset(&stAMNC, 0, sizeof(stAMNC));

    stAMNC.Color = 'W';
    m_ActorPod->Forward(rstMPK.From(), {MPK_NAMECOLOR, stAMNC}, rstMPK.ID());
}
