/*
 * =====================================================================================
 *
 *       Filename: regionmonitorop.cpp
 *        Created: 05/03/2016 19:59:02
 *  Last Modified: 05/06/2016 00:19:48
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

// handle move request from active object
// input : rstMPK
// output: response to the requestor
//
// HOWTO
// 0. if someone else is moving, reject
// 1. if the object didn't request to move out of current region, then handle it
//    and return MPK_OK or MPK_ERROR
// 2. if the object request to move out, return the destination address
//
void RegionMonitor::On_MPK_TRYMOVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    if(m_MoveRequest.Valid()){
        // ooops someone else is doing move request
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
    }else{
        AMTryMove stAMTM;
        std::memcpy(&stAMTM, rstMPK.Data(), sizeof(stAMTM));

        if(stAMTM.MapID == m_MapID){
            if(PointInRectangle(stAMTM.CurrX, stAMTM.CurrY, m_X, m_Y, m_W, m_H)){
                // ok, this region is which the requestor stays in
                if(PointInRectangle(stAMTM.X, stAMTM.Y, m_X, m_Y, m_W, m_H)){
                    // moving inside current region
                }else if(PointInRectangle(stAMTM.X, stAMTM.Y,
                            m_X - m_W, m_Y - m_H, 3 * m_W, 3 * m_H)){
                    // local move, we are still ok
                }else{
                    AMQueryRMAddress stAMQRA;
                    stAMQRA.X = stAMTM.X;
                    stAMQRA.Y = stAMTM.Y;
                    stAMQRA.MapID = stAMTM.MapID;

                    auto fnROP = [this, rstMPK, rstFromAddr](const MessagePack &rstRMPK, const Theron::Address &){
                        m_ActorPod->Forward({MPK_ADDRESS, rstRMPK.Data(), rstRMPK.DataLen()}, rstFromAdd, rstMPK.ID());
                    };
                    m_ActorPod->Forward({MPK_QUERYRMADDRESS, stAMQRA}, m_MapAddress, fnROP);
                }
            }else{
                // this region is not the region that the object stays inside
                if(PointInRectangle(stAMTM.X, stAMTM.Y, m_X, m_Y, m_W, m_H)){
                    // object is trying to move into current region
                }else if()

            }
        }else{
        }








        if(stAMTM.MapID != m_MapID){
            // this is a space move for sure
            AMQueryRMAddress stAMQRA;
            stAMQRA.MapID = stAMTM.MapID;
            stAMQRA.X = stAMTM.X;
            stAMQRA.Y = stAMTM.Y;

            auto fnQuery = [this, rstMPK, rstFromAddr](const MessagePack &rstRMPK, const Theron::Address &){
                if(rstRMPK.Type() == MPK_ADDRESS){
                    m_ActorPod->Forward({MPK_ADDRESS, rstRMPK.Data(), rstRMPK.DataLen()}, rstFromAddr, rstMPK.ID());
                }else{
                    m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                }
            };
            m_ActorPod->Forward({MPK_QUERYRMADDRESS, stAMQRA}, m_MapAddress, fnQuery);
            return;
        }

            if(m_ServiceCoreAddress == Theron::Address::Null()){
                auto fnROP = [this](const MessagePack &rstRMPK, const Theron::Address &){
                    m_ActorPod->Forward(rstMPK
                }
                m_ActorPod->Forward(MPK_QUERYSCADDRESS, m_MapAddress, fnROP);
            }


        // asking move out
        if(bSrcIn && !bDstIn){
            if(PointInRectangle(
                        m_MoveRequest.X, m_MoveRequest.Y, m_X - m_W, m_Y - m_H, 3 * m_W, 3 * m_H)){
                // local move
                int nDX = (m_MoveRequest.X - (m_X - m_W)) / m_W;
                int nDY = (m_MoveRequest.Y - (m_Y - m_H)) / m_H;

                std::string szAddr = m_NeighborV2D[nDY][nDX].PodAddress.AsString();
                m_ActorPod->Forward({MPK_ADDRESS,
                        szAddr.c_str(), szAddr.size()}, rstFromAdd, rstMPK.ID());
            }else{
                // non-local move, we have to ask help from ServerMap
            }


        // 1. prepare m_MoveRequest
        m_MoveRequest.Type = OBJECT_UNKNOWN;
        m_MoveRequest.UID = stAMTM.UID;
        m_MoveRequest.AddTime = stAMTM.AddTime;
        m_MoveRequest.Data = nullptr;
        m_MoveRequest.MPKID = rstMPK.ID();
        m_MoveRequest.X = stAMTM.X;
        m_MoveRequest.Y = stAMTM.Y;
        m_MoveRequest.R = stAMTM.R;
        m_MoveRequest.PodAddress = rstFromAddr;
        m_MoveRequest.In = PointInRectangle(stAMTM.X, stAMTM.Y, m_X, m_Y, m_W, m_H);
        m_MoveRequest.CurrIn = PointInRectangle(stAMTM.CurrX, stAMTM.CurrY, m_X, m_Y, m_W, m_H);

        // 2. do move request
        DoMoveRequest();
    }
}
