/*
 * =====================================================================================
 *
 *       Filename: monsterop.cpp
 *        Created: 05/03/2016 21:49:38
 *  Last Modified: 05/18/2017 23:15:23
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

void Monster::On_MPK_ACTION(const MessagePack &rstMPK, const Theron::Address &)
{
    AMAction stAMA;
    std::memcpy(&stAMA, rstMPK.Data(), sizeof(stAMA));

    if(stAMA.UID != UID()){
        extern MonoServer *g_MonoServer;
        m_LocationRecord[stAMA.UID].UID        = stAMA.UID;
        m_LocationRecord[stAMA.UID].MapID      = stAMA.MapID;
        m_LocationRecord[stAMA.UID].RecordTime = g_MonoServer->GetTimeTick();
        m_LocationRecord[stAMA.UID].X          = stAMA.EndX;
        m_LocationRecord[stAMA.UID].Y          = stAMA.EndY;

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

void Monster::On_MPK_ATTACK(const MessagePack &rstMPK, const Theron::Address &)
{
    AMAction stAMA;
    std::memcpy(&stAMA, rstMPK.Data(), sizeof(stAMA));

    AddTarget(stAMA.UID);
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
