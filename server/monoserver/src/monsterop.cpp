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

    if(stAMA.UID != UID()){
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
                {
                    nX = stAMA.X;
                    nY = stAMA.Y;
                    break;
                }
            case ACTION_DIE:
                {
                    RemoveTarget(stAMA.UID);
                    return;
                }
            default:
                {
                    return;
                }
        }

        extern MonoServer *g_MonoServer;
        m_LocationList[stAMA.UID] = COLocation
        {
            stAMA.UID,
            stAMA.MapID,
            g_MonoServer->GetTimeTick(),
            nX,
            nY,
            nDir,
        };

        if(InRange(RANGE_VISIBLE, stAMA.X, stAMA.Y)){
            switch(GetState(STATE_ATTACKMODE)){
                case STATE_ATTACKMODE_NORMAL:
                    {
                        if(UIDFunc::GetUIDType(stAMA.UID) == UID_PLY){
                            AddTarget(stAMA.UID);
                        }
                        break;
                    }
                case STATE_ATTACKMODE_DOGZ:
                    {
                        if(UIDFunc::GetUIDType(stAMA.UID) == UID_MON){
                            AddTarget(stAMA.UID);
                        }
                        break;
                    }
                case STATE_ATTACKMODE_ATTACKALL:
                    {
                        if(auto nType = UIDFunc::GetUIDType(stAMA.UID); nType == UID_PLY || nType == UID_MON){
                            AddTarget(stAMA.UID);
                        }
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }
    }
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
                AddTarget(stAMAK.UID);
                DispatchAction(ActionHitted(X(), Y(), Direction()));

                AddHitterUID(stAMAK.UID, stAMAK.Damage);
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

void Monster::On_MPK_NOTIFYDEAD(const MessagePack &rstMPK)
{
    AMNotifyDead stAMND;
    std::memcpy(&stAMND, rstMPK.Data(), sizeof(stAMND));

    RemoveTarget(stAMND.UID);
    m_LocationList.erase(stAMND.UID);
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

void Monster::On_MPK_CHECKMASTER(const MessagePack &)
{
}
