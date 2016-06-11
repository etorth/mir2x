/*
 * =====================================================================================
 *
 *       Filename: monsterop.cpp
 *        Created: 05/03/2016 21:49:38
 *  Last Modified: 06/11/2016 02:41:24
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

#include "monster.hpp"
#include "actorpod.hpp"
#include "mathfunc.hpp"
#include "monoserver.hpp"

void Monster::On_MPK_HI(const MessagePack &, const Theron::Address &rstFromAddr)
{
    m_RMAddress = rstFromAddr;

    QuerySCAddress();
    QueryMapAddress();
}

void Monster::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    Update();
}

void Monster::On_MPK_UPDATECOINFO(const MessagePack &rstMPK, const Theron::Address &)
{
    AMUpdateCOInfo stAMUCOI;
    std::memcpy(&stAMUCOI, rstMPK.Data(), sizeof(stAMUCOI));
    if(stAMUCOI.UID && stAMUCOI.AddTime && stAMUCOI.MapID && stAMUCOI.SessionID
            && stAMUCOI.MapID == m_MapID && stAMUCOI.X >= 0 && stAMUCOI.Y >= 0 && stAMUCOI.UID != UID() && stAMUCOI.AddTime != AddTime()){
        ReportCORecord(stAMUCOI.SessionID);
    }
}
