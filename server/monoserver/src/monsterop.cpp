/*
 * =====================================================================================
 *
 *       Filename: monsterop.cpp
 *        Created: 05/03/2016 21:49:38
 *  Last Modified: 08/11/2017 14:49:35
 *
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

void Monster::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    Update();
}

void Monster::On_MPK_PULLCOINFO(const MessagePack &rstMPK, const Theron::Address &)
{
    AMPullCOInfo stAMPCOI;
    std::memcpy(&stAMPCOI, rstMPK.Data(), sizeof(stAMPCOI));

    ReportCORecord(stAMPCOI.SessionID);
}

void Monster::On_MPK_EXP(const MessagePack &rstMPK, const Theron::Address &)
{
    if(MasterUID()){
        extern MonoServer *g_MonoServer;
        if(auto stRecord = g_MonoServer->GetUIDRecord(MasterUID())){
            m_ActorPod->Forward({rstMPK.Type(), rstMPK.Data(), rstMPK.DataLen()}, stRecord.Address);
        }else{
            GoDie();
        }
    }
}

void Monster::On_MPK_ACTION(const MessagePack &rstMPK, const Theron::Address &)
{
    AMAction stAMA;
    std::memcpy(&stAMA, rstMPK.Data(), sizeof(stAMA));

    if(stAMA.UID != UID()){
        int nX = -1;
        int nY = -1;
        switch(stAMA.Action){
            case ACTION_STAND:
            case ACTION_MOVE:
            case ACTION_ATTACK:
            case ACTION_UNDERATTACK:
                {
                    nX = stAMA.X;
                    nY = stAMA.Y;
                    break;
                }
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
        m_LocationRecord[stAMA.UID] = COLocation
        {
            stAMA.UID,
            stAMA.MapID,
            g_MonoServer->GetTimeTick(),
            nX,
            nY,
            stAMA.Direction
        };

        if(InRange(RANGE_VISIBLE, stAMA.X, stAMA.Y)){
            extern MonoServer *g_MonoServer;
            if(auto stRecord = g_MonoServer->GetUIDRecord(stAMA.UID)){
                switch(GetState(STATE_ATTACKMODE)){
                    case STATE_ATTACKMODE_NORMAL:
                        {
                            if(stRecord.ClassFrom<Player>()){ AddTarget(stAMA.UID); }
                            break;
                        }
                    case STATE_ATTACKMODE_DOGZ:
                        {
                            if(stRecord.ClassFrom<Monster>()){ AddTarget(stAMA.UID); }
                            break;
                        }
                    case STATE_ATTACKMODE_ATTACKALL:
                        {
                            if(false
                                    || stRecord.ClassFrom<Player>()
                                    || stRecord.ClassFrom<Monster>()){ AddTarget(stAMA.UID); }
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
}

void Monster::On_MPK_ATTACK(const MessagePack &rstMPK, const Theron::Address &rstAddress)
{
    AMAttack stAMAK;
    std::memcpy(&stAMAK, rstMPK.Data(), sizeof(stAMAK));

    switch(GetState(STATE_DEAD)){
        case 0:
            {
                AddTarget(stAMAK.UID);
                DispatchAction({ACTION_UNDERATTACK, 0, Direction(), X(), Y(), MapID()});

                AddHitterUID(stAMAK.UID, stAMAK.Damage);
                StruckDamage({stAMAK.UID, stAMAK.Type, stAMAK.Damage, stAMAK.Element, stAMAK.Effect});
                break;
            }
        default:
            {
                AMNotifyDead stAMND;

                stAMND.UID = UID();
                m_ActorPod->Forward({MPK_NOTIFYDEAD, stAMND}, rstAddress);
                break;
            }
    }
}

void Monster::On_MPK_MAPSWITCH(const MessagePack &, const Theron::Address &)
{
}

void Monster::On_MPK_QUERYLOCATION(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMLocation stAML;
    stAML.UID       = UID();
    stAML.MapID     = MapID();
    stAML.X         = X();
    stAML.Y         = Y();
    stAML.Direction = Direction();

    m_ActorPod->Forward({MPK_LOCATION, stAML}, rstFromAddr, rstMPK.ID());
}

void Monster::On_MPK_UPDATEHP(const MessagePack &, const Theron::Address &)
{
}

void Monster::On_MPK_BADACTORPOD(const MessagePack &, const Theron::Address &)
{
}

void Monster::On_MPK_NOTIFYDEAD(const MessagePack &rstMPK, const Theron::Address &)
{
    AMNotifyDead stAMND;
    std::memcpy(&stAMND, rstMPK.Data(), sizeof(stAMND));

    RemoveTarget(stAMND.UID);
    m_LocationRecord.erase(stAMND.UID);
}
