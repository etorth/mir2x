/*
 * =====================================================================================
 *
 *       Filename: regionmonitorop.cpp
 *        Created: 05/03/2016 19:59:02
 *  Last Modified: 05/04/2016 00:54:35
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
#include "regionmonitor.hpp"

void RegionMonitor::On_MPK_NEWMONSTER(
        const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMNewMonster stAMNM;
    std::memcpy(&stAMNM, rstMPK.Data(), sizeof(stAMNM));

    Monster *pNewMonster = (Monster *)stAMNM.Data;
    // only keep the address
    // when master want to kill it or the slave itself want to suicide
    // it send its ``this" pointer to the master
    m_CharObjectAddressL.push_back(pNewMonster->Activate());
    m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
}

void RegionMonitor::On_MPK_INITREGIONMONITOR(
        const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMRegion stAMRegion;
    std::memcpy(&stAMRegion, rstMPK.Data(), sizeof(stAMRegion));

    m_X = stAMRegion.X;
    m_Y = stAMRegion.Y;
    m_W = stAMRegion.W;
    m_H = stAMRegion.H;

    m_LocX = stAMRegion.LocX;
    m_LocY = stAMRegion.LocY;

    m_RegionDone = true;
    if(m_RegionDone && m_NeighborDone){
        AMRegionMonitorReady stReady;
        stReady.LocX = m_LocX;
        stReady.LocY = m_LocY;
        m_ActorPod->Forward(MessageBuf(MPK_REGIONMONITORREADY, stReady), rstFromAddr);
    }
}

void RegionMonitor::On_MPK_NEIGHBOR(
        const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    char *pAddr = (char *)rstMPK.Data();
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            size_t nLen = std::strlen(pAddr);
            if(nLen == 0 || (nX == 1 && nY == 1)){
                m_NeighborV2D[nY][nX] = Theron::Address::Null();
            }else{
                m_NeighborV2D[nY][nX] = Theron::Address(pAddr);
            }
            pAddr += (1 + nLen);
        }
    }

    m_NeighborDone = true;
    if(m_RegionDone && m_NeighborDone){
        AMRegionMonitorReady stReady;
        stReady.LocX = m_LocX;
        stReady.LocY = m_LocY;
        m_ActorPod->Forward(MessageBuf(MPK_REGIONMONITORREADY, stReady), rstFromAddr);
    }
}

void RegionMonitor::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    for(auto &rstAddress: m_CharObjectAddressL){
        m_ActorPod->Forward(MPK_METRONOME, rstAddress);
    }
}

void RegionMonitor::On_MPK_TRYMOVE(const MessagePack &rsgMPK, const Theron::Address &rstFromAddr)
{
    AMTryMove stMPKTM;
    std::memcpy(&stMPKTM, rstMPK.Data(), sizeof(stMPKTM));

    auto fnROP = [this](const MessagePack &rstMPK, const Theron::Address &rstAddress){
        m_SuspendMove = false;
    };

    m_ActorPod->Forward(MPK_MOVEOK, rstFromAddr, rstMPK.ID());
}
