/*
 * =====================================================================================
 *
 *       Filename: monsterop.cpp
 *        Created: 05/03/2016 21:49:38
 *  Last Modified: 05/10/2016 17:14:59
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

void Monster::On_MPK_HI(const MessagePack &, const Theron::Address &rstFromAddr)
{
    m_RMAddress = rstFromAddr;
}

void Monster::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    Update();
}

void Monster::On_MPK_MOVEOK(const MessagePack & rstMPK, const Theron::Address &)
{
    AMMoveOK stMPKMOK;
    std::memcpy(&stMPKMOK, rstMPK.Data(), sizeof(stMPKMOK));

    // 1. do the move
    m_CurrX = stMPKMOK.X;
    m_CurrY = stMPKMOK.Y;
    SetState(STATE_WAITMOVE, false);

    // 2. commit the move to the monitor
    AMCommitMove stAMCM;
    stAMCM.X = m_CurrX;
    stAMCM.Y = m_CurrY;

    m_ActorPod->Forward({MPK_COMMITMOVE, stAMCM}, m_RMAddress);
}

void Monster::On_MPK_LOCATIION(const MessagePack &rstMPK, const Theron::Address &rstAddress)
{
    AMLocation stAML;
    std::memcpy(&stAML, rstMPK.Data(), sizeof(stAML));

    int nRange = Range(RANGE_VIEW);
    if(LDistance2(stAML.X, stAML.Y, X(), Y()) > nRange * nRange){
        m_NeighborV.erase(std::remove(
                    m_NeighborV.begin(), m_NeighborV.end(), rstAddress));
    }else{
        // TODO
        // else we use this info to get target
    }
}

void Monster::On_MPK_MASTERPERSONA(const MessagePack &rstMPK, const Theron::Address &)
{
    AMMasterPersona stAMMP;
    std::memcpy(&stAMMP, rstMPK.Data(), sizeof(stAMMP));
}
