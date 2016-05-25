/*
 * =====================================================================================
 *
 *       Filename: regionmonitorop.cpp
 *        Created: 05/03/2016 19:59:02
 *  Last Modified: 05/24/2016 19:34:59
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
#include "monoserver.hpp"
#include "reactobject.hpp"
#include "regionmonitor.hpp"

void RegionMonitor::On_MPK_LEAVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMLeave stAML;
    std::memcpy(&stAML, rstMPK.Data(), rstMPK.DataLen());

    for(auto &rstRecord: m_CharObjectRecordV){
        if(rstRecord.UID == stAML.UID && rstRecord.AddTime == stAML.AddTime){
            std::swap(rstRecord, m_CharObjectRecordV.back());
            m_CharObjectRecordV.pop_back();
            break;
        }
    }

    // commit the leave, then the object can move into another RM
    m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
}

void RegionMonitor::On_MPK_CHECKCOVER(
        const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    if(m_MoveRequest.Freezed()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    AMCheckCover stAMCC;
    std::memcpy(&stAMCC, rstMPK.Data(), sizeof(stAMCC));

    if(!CoverValid(stAMCC.UID, stAMCC.AddTime, stAMCC.X, stAMCC.Y, stAMCC.R)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    m_MoveRequest.Clear();
    m_MoveRequest.CoverCheck = true;
    m_MoveRequest.Freeze();

    auto fnROP = [this](const MessagePack &, const Theron::Address &){
        // cover check requestor should response to clear the lock
        m_MoveRequest.Clear();
    };
    m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnROP);
}

void RegionMonitor::On_MPK_NEWMONSTER(
        const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    if(m_MoveRequest.Freezed()){
        // ooops someone else is doing move request
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    AMNewMonster stAMNM;
    std::memcpy(&stAMNM, rstMPK.Data(), sizeof(stAMNM));

    // for MPK_NEWMONSTER we don't have to check whether the object is to be
    // inside current region, when ServerMap do the routine, it makes sure
    if(!CoverValid(stAMNM.UID, stAMNM.AddTime, stAMNM.X, stAMNM.Y, stAMNM.R)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // check if I am only in current region?
    int nObjX = stAMNM.X - stAMNM.R;
    int nObjY = stAMNM.Y - stAMNM.R;
    int nObjW = 2 * stAMNM.R;
    int nObjH = 2 * stAMNM.R;
    if(RectangleInside(m_X, m_Y, m_W, m_H, nObjX, nObjY, nObjW, nObjH)){
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
        return;
    }

    // oooops, have to ask neighbor's opinion for cover
    // not only in the single region, then need cover check
    // all needed RM are qualified?
    bool bAllQualified = true;
    for(size_t nY = 0; nY < 3 && bAllQualified; ++nY){
        for(size_t nX = 0; nX < 3 && bAllQualified; ++nX){
            if(nX == 1 && nY == 1){ continue; }

            int nNbrX = ((int)nX - 1) * m_W + m_X;
            int nNbrY = ((int)nY - 1) * m_H + m_Y;

            // ok, no overlap, skip it
            if(CircleRectangleOverlap(stAMNM.X, stAMNM.Y, stAMNM.R, nNbrX, nNbrY, m_W, m_H)
                    && m_NeighborV2D[nY][nX].PodAddress == Theron::Address::Null()){
                bAllQualified = false;
                break;
            }
        }
    }

    // some needed neighbors are not qualified
    // then we need to inform the ServerMap who creates this monster
    if(!bAllQualified){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // prepare the MoveRequest
    m_MoveRequest.Clear();

    m_MoveRequest.Data          = stAMNM.Data;
    m_MoveRequest.X             = stAMNM.X;
    m_MoveRequest.Y             = stAMNM.Y;
    m_MoveRequest.R             = stAMNM.R;
    m_MoveRequest.UID           = stAMNM.UID;
    m_MoveRequest.AddTime       = stAMNM.AddTime;
    m_MoveRequest.MPKID         = rstMPK.ID();
    m_MoveRequest.PodAddress    = rstFromAddr;
    m_MoveRequest.CurrIn        = false;
    m_MoveRequest.OnlyIn        = false;
    m_MoveRequest.NeighborCheck = true;

    m_MoveRequest.Freeze();

    // send cover check to neighbors
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            // skip this...
            if(nX == 1 && nY == 1){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            int nNbrX = ((int)nX - 1) * m_W + m_X;
            int nNbrY = ((int)nY - 1) * m_H + m_Y;

            // no overlap, skip it
            if(!CircleRectangleOverlap(stAMNM.X, stAMNM.Y, stAMNM.R, nNbrX, nNbrY, m_W, m_H)){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            // qualified, send the cover check request

            auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                if(rstRMPK.Type() == MPK_OK){
                    m_NeighborV2D[nY][nX].Query = QUERY_OK;
                    m_NeighborV2D[nY][nX].MPKID = rstRMPK.ID(); // used when cancel the freeze
                }else{
                    m_NeighborV2D[nY][nX].Query = QUERY_ERROR;
                }
            };

            AMCheckCover stAMCC;

            // since we know the object is in current region
            // we don't have to pass (UID, AddTime) to neighbors
            stAMCC.UID = stAMNM.UID;
            stAMCC.AddTime = stAMNM.AddTime;
            stAMCC.X = stAMNM.X;
            stAMCC.Y = stAMNM.Y;
            stAMCC.R = stAMNM.R;

            m_ActorPod->Forward({MPK_CHECKCOVER, stAMCC}, m_NeighborV2D[nY][nX].PodAddress, fnROP);

            // set current state to be pending
            m_NeighborV2D[nY][nX].Query = QUERY_PENDING;
        }
    }
    // I have send MPK_CHECKCOVER to all qualified neighbors
    // now just wait for the response
    return;
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

// object send a try move request, if it's a local move, handle it
// otherwise respond with the proper remote address
void RegionMonitor::On_MPK_TRYMOVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    if(m_MoveRequest.Freezed()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    AMTryMove stAMTM;
    std::memcpy(&stAMTM, rstMPK.Data(), sizeof(stAMTM));

    if(false
            || stAMTM.MapID != m_MapID // no the same map ...
            || !PointInRectangle(stAMTM.X, stAMTM.Y, m_X - m_W, m_Y - m_H, 3 * m_W, 3 * m_H)){
        // this is a space move
        // ask ServerMap for proper RM address, no freeze here
        AMQueryRMAddress stAMQRA;
        stAMQRA.X     = stAMTM.X;
        stAMQRA.Y     = stAMTM.Y;
        stAMQRA.MapID = stAMTM.MapID;

        auto fnROP = [this, nMPKID = rstMPK.ID(), rstFromAddr](
                const MessagePack &rstRMPK, const Theron::Address &){
            m_ActorPod->Forward({MPK_ADDRESS,
                    rstRMPK.Data(), rstRMPK.DataLen()}, rstFromAddr, nMPKID);
        };
        // just return the requestor the proper address
        m_ActorPod->Forward({MPK_QUERYRMADDRESS, stAMQRA}, m_MapAddress, fnROP);
        return;
    }

    // ok it's a local move request

    // object is trying to move outside of the current region
    if(!PointInRectangle(stAMTM.X, stAMTM.Y, m_X, m_Y, m_W, m_H)){
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
    }

    // ok object is currently in this RM, and its requested distnation
    // is also in current RM

    // cover check failed in current RM, return directly
    if(!CoverValid(stAMTM.UID, stAMTM.AddTime, stAMTM.X, stAMTM.Y, stAMTM.R)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // check if the object is only in current region?
    int nObjX = stAMTM.X - stAMTM.R;
    int nObjY = stAMTM.Y - stAMTM.R;
    int nObjW = 2 * stAMTM.R;
    int nObjH = 2 * stAMTM.R;

    if(RectangleInside(m_X, m_Y, m_W, m_H, nObjX, nObjY, nObjW, nObjH)){
        // ok you are just only in current region
        // have to freeze this region since we need to wait object's response

        // 1. put the needed info for this OnlyIn
        m_MoveRequest.Clear();

        m_MoveRequest.UID     = stAMTM.UID;
        m_MoveRequest.AddTime = stAMTM.AddTime;
        m_MoveRequest.X       = stAMTM.X;
        m_MoveRequest.Y       = stAMTM.Y;
        m_MoveRequest.OnlyIn  = true;

        m_MoveRequest.Freeze();

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

    // whether I need the cover check, this is a complex function
    // the avoid using goto
    bool bAllQualified = true;
    for(size_t nY = 0; nY < 3 && bAllQualified; ++nY){
        for(size_t nX = 0; nX < 3 && bAllQualified; ++nX){
            if(nX == 1 && nY == 1){ continue; }

            int nNbrX = ((int)nX - 1) * m_W + m_X;
            int nNbrY = ((int)nY - 1) * m_H + m_Y;

            // ok, no overlap, skip it
            if(CircleRectangleOverlap(stAMTM.X, stAMTM.Y, stAMTM.R, nNbrX, nNbrY, m_W, m_H)
                    && m_NeighborV2D[nY][nX].PodAddress == Theron::Address::Null()){
                bAllQualified = false;
                break;
            }
        }
    }

    // some needed neighbors are not qualified
    if(!bAllQualified){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // prepare the MoveRequest
    m_MoveRequest.Clear();

    m_MoveRequest.X             = stAMTM.X;
    m_MoveRequest.Y             = stAMTM.Y;
    m_MoveRequest.R             = stAMTM.R;
    m_MoveRequest.UID           = stAMTM.UID;
    m_MoveRequest.AddTime       = stAMTM.AddTime;
    m_MoveRequest.MPKID         = rstMPK.ID();
    m_MoveRequest.PodAddress    = rstFromAddr;
    m_MoveRequest.CurrIn        = true;
    m_MoveRequest.OnlyIn        = false;
    m_MoveRequest.NeighborCheck = true;

    m_MoveRequest.Freeze();

    // send cover check to neighbors
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            // skip this...
            if(nX == 1 && nY == 1){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            int nNbrX = ((int)nX - 1) * m_W + m_X;
            int nNbrY = ((int)nY - 1) * m_H + m_Y;

            // no overlap, skip it
            if(!CircleRectangleOverlap(stAMTM.X, stAMTM.Y, stAMTM.R, nNbrX, nNbrY, m_W, m_H)){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            // qualified, send the cover check request

            auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                if(rstRMPK.Type() == MPK_OK){
                    m_NeighborV2D[nY][nX].Query = QUERY_OK;
                    m_NeighborV2D[nY][nX].MPKID = rstRMPK.ID(); // used when cancel the freeze
                }else{
                    m_NeighborV2D[nY][nX].Query = QUERY_ERROR;
                }
            };

            AMCheckCover stAMCC;

            // since we know the object is in current region
            // we don't have to pass (UID, AddTime) to neighbors
            stAMCC.UID = stAMTM.UID;
            stAMCC.AddTime = stAMTM.AddTime;
            stAMCC.X = stAMTM.X;
            stAMCC.Y = stAMTM.Y;
            stAMCC.R = stAMTM.R;

            m_ActorPod->Forward({MPK_CHECKCOVER, stAMCC}, m_NeighborV2D[nY][nX].PodAddress, fnROP);

            // set current state to be pending
            m_NeighborV2D[nY][nX].Query = QUERY_PENDING;
        }
    }

    // I have send MPK_CHECKCOVER to all qualified neighbors
    // now just wait for the response
}

void RegionMonitor::On_MPK_TRYSPACEMOVE(
        const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    if(m_MoveRequest.Freezed()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    AMTrySpaceMove stAMTSM;
    std::memcpy(&stAMTSM, rstMPK.Data(), sizeof(stAMTSM));

    // TODO
    // put some logic here to make sure we got into the correct RM

    // object is trying to move into current region
    if(!CoverValid(stAMTSM.UID, stAMTSM.AddTime, stAMTSM.X, stAMTSM.Y, stAMTSM.R)){
        // this request is sent from the object
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // cover is valid, check if I would only be in current region?
    int nObjX = stAMTSM.X - stAMTSM.R;
    int nObjY = stAMTSM.Y - stAMTSM.R;
    int nObjW = 2 * stAMTSM.R;
    int nObjH = 2 * stAMTSM.R;

    if(RectangleInside(m_X, m_Y, m_W, m_H, nObjX, nObjY, nObjW, nObjH)){
        // ok you would only be in this region
        // have to lock this region since we need to wait object's response

        // 1. put needed info here
        m_MoveRequest.Clear();

        m_MoveRequest.UID     = stAMTSM.UID;
        m_MoveRequest.AddTime = stAMTSM.AddTime;
        m_MoveRequest.X       = stAMTSM.X;
        m_MoveRequest.Y       = stAMTSM.Y;
        m_MoveRequest.R       = stAMTSM.R;
        m_MoveRequest.OnlyIn  = true;

        m_MoveRequest.Freeze();

        auto fnROP = [this](const MessagePack &rstRMPK, const Theron::Address &rstAddr){
            if(rstRMPK.ID() == MPK_OK){
                CharObjectRecord stCORecord;
                stCORecord.X = m_MoveRequest.X;
                stCORecord.Y = m_MoveRequest.Y;
                stCORecord.R = m_MoveRequest.R;

                stCORecord.UID        = m_MoveRequest.UID;
                stCORecord.AddTime    = m_MoveRequest.AddTime;
                stCORecord.PodAddress = rstAddr;

                // put the new record into current region
                m_CharObjectRecordV.push_back(stCORecord);
            }
            m_MoveRequest.Clear();

            // TODO
            // broadcast this new object
        };
        m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnROP);
        return;
    }

    // not only in the single region, then need cover check
    // all needed RM are qualified?
    bool bAllQualified = true;
    for(size_t nY = 0; nY < 3 && bAllQualified; ++nY){
        for(size_t nX = 0; nX < 3 && bAllQualified; ++nX){
            if(nX == 1 && nY == 1){ continue; }

            int nNbrX = ((int)nX - 1) * m_W + m_X;
            int nNbrY = ((int)nY - 1) * m_H + m_Y;

            // ok, no overlap, skip it
            if(CircleRectangleOverlap(stAMTSM.X, stAMTSM.Y, stAMTSM.R, nNbrX, nNbrY, m_W, m_H)
                    && m_NeighborV2D[nY][nX].PodAddress == Theron::Address::Null()){
                bAllQualified = false;
                break;
            }
        }
    }

    // some needed neighbors are not qualified
    if(!bAllQualified){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // prepare the MoveRequest
    m_MoveRequest.Clear();

    m_MoveRequest.X             = stAMTSM.X;
    m_MoveRequest.Y             = stAMTSM.Y;
    m_MoveRequest.R             = stAMTSM.R;
    m_MoveRequest.UID           = stAMTSM.UID;
    m_MoveRequest.AddTime       = stAMTSM.AddTime;
    m_MoveRequest.MPKID         = rstMPK.ID();
    m_MoveRequest.PodAddress    = rstFromAddr;
    m_MoveRequest.CurrIn        = false;
    m_MoveRequest.OnlyIn        = false;
    m_MoveRequest.NeighborCheck = true;

    m_MoveRequest.Freeze();

    // send cover check to neighbors
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            // skip this...
            if(nX == 1 && nY == 1){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            int nNbrX = ((int)nX - 1) * m_W + m_X;
            int nNbrY = ((int)nY - 1) * m_H + m_Y;

            // no overlap, skip it
            if(!CircleRectangleOverlap(stAMTSM.X, stAMTSM.Y, stAMTSM.R, nNbrX, nNbrY, m_W, m_H)){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            // qualified, send the cover check request

            auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                if(rstRMPK.Type() == MPK_OK){
                    m_NeighborV2D[nY][nX].Query = QUERY_OK;
                    m_NeighborV2D[nY][nX].MPKID = rstRMPK.ID(); // used when cancel the freeze
                }else{
                    m_NeighborV2D[nY][nX].Query = QUERY_ERROR;
                }
            };

            AMCheckCover stAMCC;

            // since we know the object is in current region
            // we don't have to pass (UID, AddTime) to neighbors
            stAMCC.UID = stAMTSM.UID;
            stAMCC.AddTime = stAMTSM.AddTime;
            stAMCC.X = stAMTSM.X;
            stAMCC.Y = stAMTSM.Y;
            stAMCC.R = stAMTSM.R;

            m_ActorPod->Forward({MPK_CHECKCOVER, stAMCC}, m_NeighborV2D[nY][nX].PodAddress, fnROP);

            // set current state to be pending
            m_NeighborV2D[nY][nX].Query = QUERY_PENDING;
        }
    }
    // I have send MPK_CHECKCOVER to all qualified neighbors
    // now just wait for the response
}

// try to add a new player in current RM, if succeed, it make the player incarnated = false
// this won't test the object collision since you know the reason
//
// 1. test the ground is ok
// 2. add it
// 3. set incarnated = false
// 4. if moved to a valid place, set incarnated = true;
//
// during a object with state incarnated = false, it can't do anything, otherwise player can
// cheat
//
// this function won't affect any object with incarnated = true
void RegionMonitor::On_MPK_NEWPLAYER(
        const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    if(m_MoveRequest.Freezed()){
        // ooops someone else is doing move request
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    AMNewCharObject stAMNCO;
    std::memcpy(&stAMNCO, rstMPK.Data(), sizeof(stAMNCO));

    // for MPK_NEWMONSTER we don't have to check whether the object is to be
    // inside current region, when ServerMap do the routine, it makes sure
    if(!CoverValid(0, 0, stAMNCO.MapX, stAMNCO.MapY, stAMNCO.R)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // check if I am only in current region?
    int nObjX = stAMNCO.MapX - stAMNCO.R;
    int nObjY = stAMNCO.MapY - stAMNCO.R;
    int nObjW = 2 * stAMNCO.R;
    int nObjH = 2 * stAMNCO.R;
    if(RectangleInside(m_X, m_Y, m_W, m_H, nObjX, nObjY, nObjW, nObjH)){
        CharObjectRecord stCORecord;
        stCORecord.X = stAMNCO.MapX;
        stCORecord.Y = stAMNCO.MapY;
        stCORecord.R = stAMNCO.R;

        stCORecord.UID = stAMNCO.UID;
        stCORecord.AddTime = stAMNCO.AddTime;

        ((CharObject *)stAMNCO.Data)->SetR(stAMNCO.R);
        ((CharObject *)stAMNCO.Data)->SetMapID(m_MapID);
        ((CharObject *)stAMNCO.Data)->SetLocation(GetAddress(), stAMNCO.X, stAMNCO.Y);
        stCORecord.PodAddress = ((ReactObject *)stAMNCO.Data)->Activate();

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
        return;
    }

    // oooops, have to ask neighbor's opinion for cover
    // not only in the single region, then need cover check
    // all needed RM are qualified?
    bool bAllQualified = true;
    for(size_t nY = 0; nY < 3 && bAllQualified; ++nY){
        for(size_t nX = 0; nX < 3 && bAllQualified; ++nX){
            if(nX == 1 && nY == 1){ continue; }

            int nNbrX = ((int)nX - 1) * m_W + m_X;
            int nNbrY = ((int)nY - 1) * m_H + m_Y;

            // ok, no overlap, skip it
            if(CircleRectangleOverlap(stAMNCO.X, stAMNCO.Y, stAMNCO.R, nNbrX, nNbrY, m_W, m_H)
                    && m_NeighborV2D[nY][nX].PodAddress == Theron::Address::Null()){
                bAllQualified = false;
                break;
            }
        }
    }

    // some needed neighbors are not qualified
    // then we need to inform the ServerMap who creates this monster
    if(!bAllQualified){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // prepare the MoveRequest
    m_MoveRequest.Clear();

    m_MoveRequest.Data          = stAMNCO.Data;
    m_MoveRequest.X             = stAMNCO.X;
    m_MoveRequest.Y             = stAMNCO.Y;
    m_MoveRequest.R             = stAMNCO.R;
    m_MoveRequest.UID           = stAMNCO.UID;
    m_MoveRequest.AddTime       = stAMNCO.AddTime;
    m_MoveRequest.MPKID         = rstMPK.ID();
    m_MoveRequest.PodAddress    = rstFromAddr;
    m_MoveRequest.CurrIn        = false;
    m_MoveRequest.OnlyIn        = false;
    m_MoveRequest.NeighborCheck = true;

    m_MoveRequest.Freeze();

    // send cover check to neighbors
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            // skip this...
            if(nX == 1 && nY == 1){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            int nNbrX = ((int)nX - 1) * m_W + m_X;
            int nNbrY = ((int)nY - 1) * m_H + m_Y;

            // no overlap, skip it
            if(!CircleRectangleOverlap(stAMNCO.X, stAMNCO.Y, stAMNCO.R, nNbrX, nNbrY, m_W, m_H)){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            // qualified, send the cover check request

            auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                if(rstRMPK.Type() == MPK_OK){
                    m_NeighborV2D[nY][nX].Query = QUERY_OK;
                    m_NeighborV2D[nY][nX].MPKID = rstRMPK.ID(); // used when cancel the freeze
                }else{
                    m_NeighborV2D[nY][nX].Query = QUERY_ERROR;
                }
            };

            AMCheckCover stAMCC;

            // since we know the object is in current region
            // we don't have to pass (UID, AddTime) to neighbors
            stAMCC.UID = stAMNCO.UID;
            stAMCC.AddTime = stAMNCO.AddTime;
            stAMCC.X = stAMNCO.X;
            stAMCC.Y = stAMNCO.Y;
            stAMCC.R = stAMNCO.R;

            m_ActorPod->Forward({MPK_CHECKCOVER, stAMCC}, m_NeighborV2D[nY][nX].PodAddress, fnROP);

            // set current state to be pending
            m_NeighborV2D[nY][nX].Query = QUERY_PENDING;
        }
    }
    // I have send MPK_CHECKCOVER to all qualified neighbors
    // now just wait for the response
    return;
}

// add a completely new object into current region
void RegionMonitor::On_MPK_ADDCHAROBJECT(
        const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMAddCharObject stAMACO;
    std::memcpy(&stAMNCO, rstMPK.Data(), sizeof(stAMNCO));

    // routing error
    if(!In(stAMACO.MapID, stAMACO.MapX, stAMACO.MapY)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // try to be in an impossible place
    if(!GroundValid(stAMACO.MapX, stAMACO.MapY, stAMACO.R)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // try to be active in a locked RM
    if(!stAMACO.AllowVoid && m_MoveRequest.Freezed()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // try to be in a place taken by others
    if(!stAMACO.AllowVoid && !CoverValid(0, 0, stAMACO.MapX, stAMACO.MapY, stAMACO.R)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // ok now we can do something now

    // find conditions that we can finish the adding in this function
    // 1. allow void state, we even don't care about if current cover is freezed or not
    // 2. the new char object is only in current RM

    int nObjX = stAMACO.MapX - stAMACO.R;
    int nObjY = stAMACO.MapY - stAMACO.R;
    int nObjW = 2 * stAMACO.R;
    int nObjH = 2 * stAMACO.R;

    bool bAllowVoid = stAMACO.AllowVoid;
    bool bOnlyIn    = RectangleInside(m_X, m_Y, m_W, m_H, nObjX, nObjY, nObjW, nObjH);

    if(bAllowVoid || bOnlyIn){
        CharObject *pCharObject = nullptr;
        switch(stAMACO.Type){
            case OBJECT_MONSTER:
                {
                    pCharObject = new Monster(stAMACO.GeneralID);
                    break;
                }
            case OBJECT_PLAYER:
                {
                    pCharObject = new Player(stAMACO.GeneralID, stAMACO.SubID);
                    break;
                }
            default:
                {
                    break;
                }
        }

        if(!pCharObject){
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            return;
        }

        // if it's only in then we can set it as active immediately
        pCharObject->ResetState(bOnlyIn ? STATE_ACTIVE : STATE_INCARNATED, true);
        pCharObject->ResetR(stAMACO.R);
        pCharObject->Locate(m_MapID, stAMACO.MapX, stAMACO.MapY);
        pCharObject->Locate(GetAddress());

        CharObjectRecord stCORecord;
        stCORecord.X = pCharObject->X();
        stCORecord.Y = pCharObject->Y();
        stCORecord.R = pCharObject->R();

        stCORecord.UID        = pCharObject->UID();
        stCORecord.AddTime    = pCharObject->AddTime();
        stCORecord.PodAddress = pCharObject->Activate();

        m_CharObjectRecordV.push_back(stCORecord);

        // we respond to ServiceCore, but it won't respond again
        // TODO & TBD
        // here maybe protential bug:
        // 1. message to creater did arrival yet
        // 2. but the actor has already been working
        //
        m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());

        // actually here we don't need to create the RM address and send it
        // since for the receiving object, it can take the address of the Operate()
        m_ActorPod->Forward(MPK_HI, stCORecord.PodAddress);
        return;
    }

    // don't allow void state, and the object is not only in one RM
    // thing be more complicated we have to ask neighbor's opinion for cover

    // all needed RM are qualified?
    bool bAllQualified = true;
    for(size_t nY = 0; nY < 3 && bAllQualified; ++nY){
        for(size_t nX = 0; nX < 3 && bAllQualified; ++nX){
            if(nX == 1 && nY == 1){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            int nNbrX = ((int)nX - 1) * m_W + m_X;
            int nNbrY = ((int)nY - 1) * m_H + m_Y;

            // check overlap, skip it
            int nMapX = stAMACO.MapX;
            int nMapY = stAMACO.MapY;
            int nR    = stAMACO.R;

            if(CircleRectangleOverlap(nMapX, nMapY, nR, nNbrX, nNbrY, m_W, m_H)){
                if(m_NeighborV2D[nY][nX].PodAddress != Theron::Address::Null()){
                    // overlap, and qualified
                    m_NeighborV2D[nY][nX].Query = QUERY_OK;
                }else{
                    // overlap and not qualified
                    m_NeighborV2D[nY][nX].Query = QUERY_NA;
                    bAllQualified = false;
                    break;
                }
            }else{
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
            }
        }
    }

    // some needed neighbors are not qualified
    // then we need to inform the ServerMap who creates this monster
    if(!bAllQualified){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // all 3 x 3 are marked as QUERY_NA or QUERY_OK
    // prepare the MoveRequest
    m_MoveRequest.Clear();

    m_MoveRequest.Data          = nullptr;
    m_MoveRequest.X             = stAMACO.MapX;
    m_MoveRequest.Y             = stAMACO.MapY;
    m_MoveRequest.R             = stAMACO.R;
    m_MoveRequest.UID           = 0; // pending
    m_MoveRequest.AddTime       = 0; // pending
    m_MoveRequest.MPKID         = rstMPK.ID();
    m_MoveRequest.PodAddress    = rstFromAddr;
    m_MoveRequest.CurrIn        = false;
    m_MoveRequest.OnlyIn        = false;
    m_MoveRequest.NeighborCheck = true;

    m_MoveRequest.Freeze();

    // send cover check to neighbors
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            // skip those un-needed RM's
            if(m_NeighborV2D[nY][nX].Query == QUERY_NA){ continue; }

            // qualified, send the cover check request

            auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                if(rstRMPK.Type() == MPK_OK){
                    m_NeighborV2D[nY][nX].Query = QUERY_OK;
                    m_NeighborV2D[nY][nX].MPKID = rstRMPK.ID(); // used when cancel the freeze
                }else{
                    m_NeighborV2D[nY][nX].Query = QUERY_ERROR;
                }
            };

            AMCheckCover stAMCC;

            // since we know the object is in current region
            // we don't have to pass (UID, AddTime) to neighbors
            stAMCC.X       = stAMACO.MapX;
            stAMCC.Y       = stAMACO.MapY;
            stAMCC.R       = stAMACO.R;
            stAMCC.UID     = 0;
            stAMCC.AddTime = 0;

            m_ActorPod->Forward({MPK_CHECKCOVER, stAMCC}, m_NeighborV2D[nY][nX].PodAddress, fnROP);

            // set current state to be pending
            m_NeighborV2D[nY][nX].Query = QUERY_PENDING;
        }
    }
    // I have send MPK_CHECKCOVER to all qualified neighbors
    // now just wait for the response
    return;
}
