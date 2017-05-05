/*
 * =====================================================================================
 *
 *       Filename: monsterop.cpp
 *        Created: 05/03/2016 21:49:38
 *  Last Modified: 05/04/2017 17:57:11
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
        if((std::abs(stAMA.X - X()) <= SYS_MAPVISIBLEW) && (std::abs(stAMA.Y - Y()) <= SYS_MAPVISIBLEH)){
            extern MonoServer *g_MonoServer;
            if(auto stRecord = g_MonoServer->GetUIDRecord(stAMA.UID)){
                if(stRecord.ClassFrom<Player>()){
                    m_TargetInfo.UID   = stAMA.UID;
                    m_TargetInfo.MapID = stAMA.MapID;
                    m_TargetInfo.X     = stAMA.X;
                    m_TargetInfo.Y     = stAMA.Y;
                }
            }
        }
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
