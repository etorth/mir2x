/*
 * =====================================================================================
 *
 *       Filename: regionmonitorop.cpp
 *        Created: 05/03/2016 19:59:02
 *  Last Modified: 05/07/2016 14:12:12
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
#include "charobject.hpp"
#include "reactobject.hpp"
#include "regionmonitor.hpp"

void RegionMonitor::On_MPK_CHECKCOVER(
        const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMCheckCover stAMCC;
    std::memcpy(&stAMCC, rstMPK.Data(), sizeof(stAMCC));

    bool bCoverValid = CoverValid(0, 0, stAMCC.X, stAMCC.Y, stAMCC.R);
    if(bCoverValid){
        m_MoveRequest.CoverCheck = true;

        auto fnROP = [this](const MessagePack &, const Theron::Address &){
            m_MoveRequest.Clear();
        };
        m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnROP);
    }else{
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
    }
}

void RegionMonitor::On_MPK_NEWMONSTER(
        const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMNewMonster stAMNM;
    std::memcpy(&stAMNM, rstMPK.Data(), sizeof(stAMNM));

    if(m_MoveRequest.Valid()){
        // ooops someone else is doing move request
        // for move request, we should respond
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // for MPK_NEWMONSTER we don't have to check whether the object is to be
    // inside current region, when ServerMap do the routine, it makes sure

    if(!CoverValid(0, 0, stAMNM.X, stAMNM.Y, stAMNM.R)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // check if I am only in current region?
    if(RectangleInside(m_X, m_Y, m_W, m_H,
                stAMNM.X - stAMNM.R, stAMNM.Y - stAMNM.R, 2 * stAMNM.R, 2 * stAMNM.R)){
        CharObjectRecord stCORecord;
        stCORecord.X = stAMNM.X;
        stCORecord.Y = stAMNM.Y;
        stCORecord.R = stAMNM.R;

        stCORecord.UID = stAMNM.UID;
        stCORecord.AddTime = stAMNM.AddTime;

        ((CharObject *)stAMNM.Data)->SetR(stAMNM.R);
        ((CharObject *)stAMNM.Data)->SetMapID(m_MapID);
        ((CharObject *)stAMNM.Data)->SetLocation(GetAddress(), stAMNM.X, stAMNM.Y);
        stCORecord.PodAddress = ((ReactObject *)stAMNM.Data)->Activate();

        m_CharObjectRecordV.push_back(stCORecord);

        // we respond to ServerMap, but it won't respond again
        // TODO & TBD
        // here maybe protential bug:
        // 1. message to creater did arrival yet
        // 2. but the actor has already been working
        //
        m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());

        // actually here we don't need to create the RM address and send it
        // since for the receiving object, it can take the address of the Operate()
        m_ActorPod->Forward(MPK_HI, stCORecord.PodAddress);

        m_MoveRequest.Clear();
        return;
    }

    // oooops, have to ask neighbor's opinion for cover
    //
    // 0. prepare the MoveRequest
    m_MoveRequest.Data = stAMNM.Data;
    m_MoveRequest.X = stAMNM.X;
    m_MoveRequest.Y = stAMNM.Y;
    m_MoveRequest.R = stAMNM.R;
    m_MoveRequest.UID = 0;
    m_MoveRequest.AddTime = 0;
    m_MoveRequest.MPKID = rstMPK.ID();
    m_MoveRequest.PodAddress = rstFromAddr;

    // 1. init all to be QUERY_NA
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            m_NeighborV2D[nY][nX].Query = QUERY_NA;
        }
    }

    // 2. check one by one
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            if(nX == 1 && nY == 1){ continue; }

            int nNeighborX = ((int)nX - 1) * m_W + m_X;
            int nNeighborY = ((int)nY - 1) * m_H + m_Y;

            if(CircleRectangleOverlap(stAMNM.X, stAMNM.Y,
                        stAMNM.R, nNeighborX, nNeighborY, m_W, m_H)){
                // this neighbor is not valid to hold any objects
                // TODO & TBD
                // here is some place I have to use trigger? for neighbor A
                // if we found it's qualified, then we send CheckCover, then
                // we found B is not qualified, we have to quit, but A is
                // locked by the CheckCover operation
                m_NeighborV2D[nY][nX].Query = QUERY_ERROR;
                // then we don't have to send this failure in the trigger
                m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                return;
            }

            // we need to ask for opinion from this neighbor
            auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                // just make a tag and trigger will do the rest
                m_NeighborV2D[nY][nX].Query = (rstRMPK.Type() == MPK_OK ? QUERY_OK : QUERY_ERROR);
                m_NeighborV2D[nY][nX].MPKID = (rstRMPK.Type() == MPK_OK ? rstRMPK.ID() : 0);
            };

            AMCheckCover stAMCC;

            stAMCC.X = stAMNM.X;
            stAMCC.Y = stAMNM.Y;
            stAMCC.R = stAMNM.R;

            m_ActorPod->Forward({MPK_CHECKCOVER, stAMCC}, m_NeighborV2D[nY][nX].PodAddress, fnROP);
        }
    }

    // I have send MPK_CHECKCOVER to all qualified neighbors
    // now just wait for the response
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

    m_MapID = stAMRegion.MapID;

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
        return;
    }

    AMTryMove stAMTM;
    std::memcpy(&stAMTM, rstMPK.Data(), sizeof(stAMTM));

    if(stAMTM.MapID != m_MapID){
        // ooops this is even a space move
        // ask your boss for the proper RM address
        AMQueryRMAddress stAMQRA;
        stAMQRA.X = stAMTM.X;
        stAMQRA.Y = stAMTM.Y;
        stAMQRA.MapID = stAMTM.MapID;

        auto fnROP = [this, rstMPK, rstFromAddr](
                const MessagePack &rstRMPK, const Theron::Address &){
            m_ActorPod->Forward({MPK_ADDRESS,
                    rstRMPK.Data(), rstRMPK.DataLen()}, rstFromAddr, rstMPK.ID());
        };
        // just return the requestor the proper address
        m_ActorPod->Forward({MPK_QUERYRMADDRESS, stAMQRA}, m_MapAddress, fnROP);
        return;
    }

    // ok it's a move on current map

    // ok, this region is which the requestor stays in
    if(PointInRectangle(stAMTM.CurrX, stAMTM.CurrY, m_X, m_Y, m_W, m_H)){
        if(PointInRectangle(stAMTM.X, stAMTM.Y, m_X, m_Y, m_W, m_H)){
            // moving inside current region
            if(!CoverValid(stAMTM.UID, stAMTM.AddTime, stAMTM.X, stAMTM.Y, stAMTM.R)){
                m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                return;
            }

            // check if I am only in current region?
            if(RectangleInside(m_X, m_Y, m_W, m_H,
                        stAMTM.X - stAMTM.R, stAMTM.Y - stAMTM.R, 2 * stAMTM.R, 2 * stAMTM.R)){
                // ok you are just only in current region
                // have to lock this region since we need to wait object's response
                //
                m_MoveRequest.UID = stAMTM.UID;
                m_MoveRequest.AddTime = stAMTM.AddTime;
                m_MoveRequest.X = stAMTM.X;
                m_MoveRequest.Y = stAMTM.Y;

                auto fnROP = [this](const MessagePack &rstRMPK, const Theron::Address &){
                    // object moved, so we need to update the location
                    if(rstRMPK.Type() == MPK_OK){
                        for(auto &rstRecord: m_CharObjectRecordV){
                            if(true
                                    && rstRecord.UID == m_MoveRequest.UID
                                    && rstRecord.AddTime == m_MoveRequest.AddTime){
                                rstRecord.X = m_MoveRequest.X;
                                rstRecord.Y = m_MoveRequest.Y;
                                break;
                            }
                        }
                    }
                    // no matter moved or not, release the current RM
                    m_MoveRequest.Clear();
                };
                m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnROP);
                return;
            }

            // because of size, have to ask neighbor's opinion for cover
            // 1. init all to be QUERY_NA
            for(size_t nY = 0; nY < 3; ++nY){
                for(size_t nX = 0; nX < 3; ++nX){
                    m_NeighborV2D[nY][nX].Query = QUERY_NA;
                }
            }

            // 2. check one by one
            for(size_t nY = 0; nY < 3; ++nY){
                for(size_t nX = 0; nX < 3; ++nX){
                    if(nX == 1 && nY == 1){ continue; }

                    int nNeighborX = ((int)nX - 1) * m_W + m_X;
                    int nNeighborY = ((int)nY - 1) * m_H + m_Y;

                    // ok, no overlap, skip it
                    if(!CircleRectangleOverlap(stAMTM.X, stAMTM.Y,
                                stAMTM.R, nNeighborX, nNeighborY, m_W, m_H)){
                        continue;
                    }

                    // overlap, then whether its qualified?
                    if(m_NeighborV2D[nY][nX].PodAddress == Theron::Address::Null()){
                        // this neighbor is not valid to hold any objects
                        // TODO & TBD
                        // here is some place I have to use trigger? for neighbor A
                        // if we found it's qualified, then we send CheckCover, then
                        // we found B is not qualified, we have to quit, but A is
                        // locked by the CheckCover operation
                        m_NeighborV2D[nY][nX].Query = QUERY_ERROR;
                        // then we don't have to send this failure in the trigger
                        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                        return;
                    }

                    // qualified, we need to ask for opinion from this neighbor
                    auto fnROP = [this, nX, nY](
                            const MessagePack &rstRMPK, const Theron::Address &){
                        // just make a tag and trigger will do the rest
                        m_NeighborV2D[nY][nX].Query = (
                                rstRMPK.Type() == MPK_OK ? QUERY_OK : QUERY_ERROR);
                    };

                    AMCheckCover stAMCC;

                    // since we know the object is in current region
                    // we don't have to pass (UID, AddTime) to neighbors
                    stAMCC.X = stAMTM.X;
                    stAMCC.Y = stAMTM.Y;
                    stAMCC.R = stAMTM.R;

                    m_ActorPod->Forward({MPK_CHECKCOVER,
                            stAMCC}, m_NeighborV2D[nY][nX].PodAddress, fnROP);
                }
            }

            // I have send MPK_CHECKCOVER to all qualified neighbors
            // now just wait for the response
            m_MoveRequest.MPKID = rstMPK.ID();
            m_MoveRequest.PodAddress = rstFromAddr;
            return;
        }else if(PointInRectangle(stAMTM.X, stAMTM.Y, m_X - m_W, m_Y - m_H, 3 * m_W, 3 * m_H)){
            // not move around in one region, but just a local move
            // return the RM address
            auto stAddress = NeighborAddress(stAMTM.X, stAMTM.Y);
            if(stAddress == Theron::Address::Null()){
                // this neighbor is not capable to holding the object
                m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                return;
            }
            std::string szAddress = stAddress.AsString();
            m_ActorPod->Forward({MPK_ADDRESS,
                    (uint8_t *)szAddress.c_str(), szAddress.size()}, rstFromAddr, rstMPK.ID());
            return;
            // OK we are done here
        }else{
            // ooops this is a long move on current map
            // ask your server map for the proper RM address
            AMQueryRMAddress stAMQRA;
            stAMQRA.X = stAMTM.X;
            stAMQRA.Y = stAMTM.Y;
            stAMQRA.MapID = stAMTM.MapID;

            auto fnROP = [this, rstMPK, rstFromAddr](
                    const MessagePack &rstRMPK, const Theron::Address &){
                m_ActorPod->Forward({MPK_ADDRESS,
                        rstRMPK.Data(), rstRMPK.DataLen()}, rstFromAddr, rstMPK.ID());
            };
            m_ActorPod->Forward({MPK_QUERYRMADDRESS, stAMQRA}, m_MapAddress, fnROP);
        }
    }else{
        // this region is not the region that the object stays inside
        if(PointInRectangle(stAMTM.X, stAMTM.Y, m_X, m_Y, m_W, m_H)){
            // object is trying to move into current region
            if(!CoverValid(stAMTM.UID, stAMTM.AddTime, stAMTM.X, stAMTM.Y, stAMTM.R)){
                m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            }
            return;
        }else{
            // object is neither staying in current region nor trying to get inside
            // so it's just a cover check broadcasted by neighbor
            m_ActorPod->Forward(CoverValid(stAMTM.UID, stAMTM.AddTime,
                        stAMTM.X, stAMTM.Y, stAMTM.R) ? MPK_OK : MPK_ERROR, rstFromAddr, rstMPK.ID());
            return;
        }
    }
}
