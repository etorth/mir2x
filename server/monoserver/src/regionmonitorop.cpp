/*
 * =====================================================================================
 *
 *       Filename: regionmonitorop.cpp
 *        Created: 05/03/2016 19:59:02
 *  Last Modified: 05/05/2016 01:30:41
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
#include "mathfunc.hpp"
#include "actorpod.hpp"
#include "reactobject.hpp"
#include "regionmonitor.hpp"

void RegionMonitor::On_MPK_NEWMONSTER(
        const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMNewMonster stAMNM;
    std::memcpy(&stAMNM, rstMPK.Data(), sizeof(stAMNM));

    if(m_MoveRequest.Valid()){
        // ooops someone else is doing move request
        // for move request, we should respond
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
    }else{
        // 1. prepare m_MoveRequest
        m_MoveRequest.Type = OBJECT_MONSTER;
        m_MoveRequest.UID = stAMNM.UID;
        m_MoveRequest.AddTime = stAMNM.AddTime;
        m_MoveRequest.Data = stAMNM.Data;
        m_MoveRequest.MPKID = rstMPK.ID();
        m_MoveRequest.PodAddress = rstFromAddr;
        m_MoveRequest.In = PointInRectangle(stAMNM.X, stAMNM.Y, m_X, m_Y, m_W, m_H);

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
                m_NeighborV2D[nY][nX].PodAddress = Theron::Address::Null();
            }else{
                m_NeighborV2D[nY][nX].PodAddress = Theron::Address(pAddr);
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
    for(auto &rstRecord: m_CharObjectRecordV){
        m_ActorPod->Forward(MPK_METRONOME, rstRecord.PodAddress);
    }
}

void RegionMonitor::On_MPK_TRYMOVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMTryMove stAMTM;
    std::memcpy(&stAMTM, rstMPK.Data(), sizeof(stAMTM));

    if(m_MoveRequest.Valid()){
        // ooops someone else is doing move request
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
    }else{
        // 1. prepare m_MoveRequest
        m_MoveRequest.Type = OBJECT_UNKNOWN;
        m_MoveRequest.UID = stAMTM.UID;
        m_MoveRequest.AddTime = stAMTM.AddTime;
        m_MoveRequest.Data = stAMTM.Data;
        m_MoveRequest.MPKID = rstMPK.ID();
        m_MoveRequest.X = stAMTM.X;
        m_MoveRequest.Y = stAMTM.Y;
        m_MoveRequest.R = stAMTM.R;
        m_MoveRequest.PodAddress = rstFromAddr;
        m_MoveRequest.In = PointInRectangle(stAMTM.X, stAMTM.Y, m_X, m_Y, m_W, m_H);

        // 2. do move request
        DoMoveRequest();
    }
}
