/*
 * =====================================================================================
 *
 *       Filename: monsterop.cpp
 *        Created: 05/03/2016 21:49:38
 *  Last Modified: 07/25/2017 11:17:39
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

void Monster::On_MPK_EXP(const MessagePack &, const Theron::Address &)
{
}

void Monster::On_MPK_ACTION(const MessagePack &rstMPK, const Theron::Address &)
{
    AMAction stAMA;
    std::memcpy(&stAMA, rstMPK.Data(), sizeof(stAMA));

    if(stAMA.UID != UID()){
        extern MonoServer *g_MonoServer;
        m_LocationRecord[stAMA.UID].UID        = stAMA.UID;
        m_LocationRecord[stAMA.UID].MapID      = stAMA.MapID;
        m_LocationRecord[stAMA.UID].RecordTime = g_MonoServer->GetTimeTick();
        m_LocationRecord[stAMA.UID].X          = stAMA.AimX;
        m_LocationRecord[stAMA.UID].Y          = stAMA.AimY;

        switch(stAMA.Action){
            case ACTION_DIE:
                {
                    RemoveTarget(stAMA.UID);
                    return;
                }
            default:
                {
                    break;
                }
        }

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

    AddHitterUID(stAMAK.UID, stAMAK.Damage);
    StruckDamage({stAMAK.UID, stAMAK.Type, stAMAK.Damage, stAMAK.Element, stAMAK.Effect});

    if(GetState(STATE_DEAD)){
        // 1. send death information
        AMAction stAMACT;
        std::memset(&stAMACT, 0, sizeof(stAMACT));

        stAMACT.UID   = UID();
        stAMACT.MapID = MapID();

        stAMACT.Action      = ACTION_DIE;
        stAMACT.ActionParam = 0;
        stAMACT.Speed       = SYS_DEFSPEED;
        stAMACT.Direction   = Direction();

        stAMACT.X    = X();
        stAMACT.Y    = Y();
        stAMACT.AimX = X();
        stAMACT.AimY = Y();

        m_ActorPod->Forward({MPK_ACTION, stAMACT}, rstAddress);

        // 2. send experience to hitters
        DispatchHitterExp();
        return;
    }

    AddTarget(stAMAK.UID);
    DispatchAction({ACTION_UNDERATTACK, 0, Direction(), X(), Y(), MapID()});

    AMUpdateHP stAMUHP;
    stAMUHP.UID   = UID();
    stAMUHP.MapID = MapID();
    stAMUHP.X     = X();
    stAMUHP.Y     = Y();
    stAMUHP.HP    = HP();
    stAMUHP.HPMax = HPMax();

    if(true
            && ActorPodValid()
            && m_Map
            && m_Map->ActorPodValid()){
        m_ActorPod->Forward({MPK_UPDATEHP, stAMUHP}, m_Map->GetAddress());
    }
}

void Monster::On_MPK_MAPSWITCH(const MessagePack &, const Theron::Address &)
{
}

void Monster::On_MPK_QUERYLOCATION(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMLocation stAML;
    stAML.UID   = UID();
    stAML.MapID = MapID();
    stAML.X     = X();
    stAML.Y     = Y();

    m_ActorPod->Forward({MPK_LOCATION, stAML}, rstFromAddr, rstMPK.ID());
}

void Monster::On_MPK_UPDATEHP(const MessagePack &, const Theron::Address &)
{
}

void Monster::On_MPK_BADACTORPOD(const MessagePack &, const Theron::Address &)
{
}
