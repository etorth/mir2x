/*
 * =====================================================================================
 *
 *       Filename: regionmonitorop.cpp
 *        Created: 05/03/2016 19:59:02
 *  Last Modified: 05/04/2016 18:58:32
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

    if(m_MoveRequest.Valid()){
        // ooops someone else is doing move request
        if(rstMPK.ID()){
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        }else{
            delete (Monster *)stAMNM.Data;
        }
    }else{
        // 1. prepare m_MoveRequest
        m_MoveRequest.Type = OBJECT_MONSTER;
        m_MoveRequest.UID = stAMNM.UID;
        m_MoveRequest.AddTime = stAMNM.AddTime;
        m_MoveRequest.Data = stAMNM.Data;
        m_MoveRequest.ID = rstMPK.ID();
        m_MoveRequest.PodAddress = rstFromAddr;

        // 2. do move request
        DoMoveRequest();
    }
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

void RegionMonitor::On_MPK_TRYMOVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMTryMove stAMTM;
    std::memcpy(&stAMTM, rstMPK.Data(), sizeof(stAMTM));

    // someone else is requesting move
    if(!m_MoveRequest.Valid()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // 1. ok you take the opportunity to move
    m_MoveRequest.ID = rstMPK.ID();
    m_MoveRequest.PodAddress = rstFromAddr;

    // 2. check collision with others
    for(auto &rstRecord: m_CharObjectRecordV){
        if(CircleOverlap(stAMTM.X, stAMTM.Y, stAMTM.R, rstRecord.X, rstRecord.Y, rstRecord.R)){
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            return;
        }
    }

    // 3. need to query neighbors?
    if(RectInside(m_X, m_Y, m_W, m_H,
                stAMTM.X - stAMTM.R, stAMTM.Y - stAMTM.R, stAMTM.R * 2, stAMTM.R * 2)){
        // no need
        m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
        return;
    }

    // 4. ok we have to query neighbors
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            if(false
                    || (nX == 1 && nY == 1)
                    || m_NeighborV2D[nY][nX] == Theron::Address::Null()){
                m_NeighborV2D[nY][nX].Query = true;
                continue;
            }
            m_NeighborV2D[nY][nX].Query = false;

            int nNeighborX = ((int)nX - 1) * m_W + m_X;
            int nNeighborY = ((int)nY - 1) * m_H + m_Y;

            if(RectangleOverlap(nNeighborX, nNeighborY, m_W, m_H, m_X, m_Y, m_W, m_H)){
                auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                    m_NeighborV2D[nY][nX].Query = (rstRMPK.Type() == MPK_OK);
                };
                m_ActorPod->Forward(rstMPK, m_NeighborV2D[nY][nX].PodAddress, fnROP);
            }
        }
    }
}
