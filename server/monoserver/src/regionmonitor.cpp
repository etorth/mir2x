/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.cpp
 *        Created: 04/22/2016 01:15:24
 *  Last Modified: 05/05/2016 22:48:34
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

#include "mathfunc.hpp"
#include "actorpod.hpp"
#include "monoserver.hpp"
#include "regionmonitor.hpp"

Theron::Address RegionMonitor::Activate()
{
    auto stAddr = Transponder::Activate();
    if(stAddr != Theron::Address::Null()){
        m_ActorPod->Forward(MPK_ACTIVATE, m_MapAddress);
    }
    return stAddr;
}

void RegionMonitor::Operate(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    switch(rstMPK.Type()){
        case MPK_TRYMOVE:
            {
                On_MPK_TRYMOVE(rstMPK, rstFromAddr);
                break;
            }
        case MPK_METRONOME:
            {
                On_MPK_METRONOME(rstMPK, rstFromAddr);
                break;
            }
        case MPK_NEWMONSTER:
            {
                On_MPK_NEWMONSTER(rstMPK, rstFromAddr);
                break;
            }
        case MPK_INITREGIONMONITOR:
            {
                On_MPK_INITREGIONMONITOR(rstMPK, rstFromAddr);
                break;
            }
        case MPK_NEIGHBOR:
            {
                On_MPK_NEIGHBOR(rstMPK, rstFromAddr);
                break;
            }
        default:
            {
                // when operating, MonoServer is ready for use
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING,
                        "unsupported message type (%d:%s)", rstMPK.Type(), rstMPK.Name());
                break;
            }
    }
}

// do moving logic w.r.t m_MoveRequest, need prepare m_MoveRequest
// w.r.t the MessagePack
//
// Input:   m_MoveRequest
// Output:  response no matter succeed or failed
//
// try to keep it simple, if someone else is moving, just reject this
// request, you can check the moving object and make sure this moving
// object won't cause collision, but that adds complexity
//
// HOWO:
// From the object's view, there are three kind of moving:
// 1. add a new, unactivated object to the current region
// 2. an object request to move inside current region
// 3. an object request to get out of current region
//
// From the region monitor's view:
// 1. get MPK_ADDMONSTOR, MPK_ADDPLAYER, MPK_TRYMOVE
// 2. this is a new object?
// 3. this object is currently in my monitored region?
// 4. this object is trying to move out?
//
// for ADDMSTER, if current monster is not activated, then where you
// create where you delete it, if activated, just kill it by its master
// actor.
void RegionMonitor::DoMoveRequest()
{
    bool bMoveOK = false;
    // 1. check if the ground is OK for current moving request
    //    no collision check between objects here
    if(!GroundValid(m_MoveRequest.X, m_MoveRequest.Y, m_MoveRequest.R)){
        bMoveOK = false;
        goto __REGIONMONITOR_DOMOVEREQUEST_DONE_1;
    }

    // 2. will I collide with any one in current region?
    for(auto &rstRecord: m_CharObjectRecordV){
        if(!m_MoveRequest.Data){
            // this is a moving for an existing object
            //
            // no matter the region that the monster is in, or
            // neighbor region, this check is always OK, since
            // (UID, AddTime) is unique
            if(true
                    && rstRecord.UID == m_MoveRequest.UID
                    && rstRecord.AddTime == m_MoveRequest.AddTime){
                continue;
            }
        }

        if(CircleOverlap(rstRecord.X, rstRecord.Y, rstRecord.R, 
                    m_MoveRequest.X, m_MoveRequest.Y, m_MoveRequest.R)){
            bMoveOK = false;
            goto __REGIONMONITOR_DOMOVEREQUEST_DONE_1;
        }
    }

    // 3. is this a moving request by broadcast?
    //    we need to test this moving request happens in current region
    //    since region monitor will broadcast moving request among its neighbors
    if(!m_MoveRequest.In){
        // via broadcast, OK we are all-set
        // reply to the regioin hoding the moving object
        bMoveOK = true;
        goto __REGIONMONITOR_DOMOVEREQUEST_DONE_1;
    }

    // 4. I am in current region
    //    check if I am only in current region?
    if(RectangleInside(m_X, m_Y, m_W, m_H,
                m_MoveRequest.X - m_MoveRequest.R,
                m_MoveRequest.Y - m_MoveRequest.R,
                2 * m_MoveRequest.R,
                2 * m_MoveRequest.R)){
        // yes only in current region
        // all-set
        bMoveOK = true;
        goto __REGIONMONITOR_DOMOVEREQUEST_DONE_1;
    }

    // 5. OK I am in current region and cover some neighbor-regions
    //    need to check neighbors
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            if(nX == 1 && nY == 1){
                m_NeighborV2D[nY][nX].Query = 1;
                continue;
            }

            int nNeighborX = ((int)nX - 1) * m_W + m_X;
            int nNeighborY = ((int)nY - 1) * m_H + m_Y;

            if(RectangleOverlap(nNeighborX, nNeighborY, m_W, m_H, m_X, m_Y, m_W, m_H)){
                if(!m_NeighborV2D[nY][nX].Valid()){
                    // ok this neighbor is not valid to hold any objects
                    bMoveOK = false;
                    goto __REGIONMONITOR_DOMOVEREQUEST_DONE_1;
                }

                // we need to ask for opinion from this neighbor
                m_NeighborV2D[nY][nX].Query = -1;
                auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                    m_NeighborV2D[nY][nX].Query = (rstRMPK.Type() == MPK_OK ? 1 : 0);
                };

                AMTryMove stAMTM;
                stAMTM.X       = m_MoveRequest.X;
                stAMTM.Y       = m_MoveRequest.Y;
                stAMTM.R       = m_MoveRequest.R;
                stAMTM.UID     = m_MoveRequest.UID;
                stAMTM.AddTime = m_MoveRequest.AddTime;

                m_ActorPod->Forward({MPK_MOVE, stAMTM}, m_NeighborV2D[nY][nX].PodAddress, fnROP);
            }else{
                m_NeighborV2D[nY][nX].Query = 1;
            }
        }
    }

    // 6. I have sent query to all my qualified neighbors, I'm done
    return;

__REGIONMONITOR_DOMOVEREQUEST_DONE_1:
    auto fnROP = [this](const MessagePack &, const Theron::Address &rstFromAddr){
        // TODO & TBD
        // do I need to put this logic out of fnROP?
        if(bMoveOK && m_MoveRequest.In){
            if(stRecord.Data){
                // this is a new object
                CharObjectRecord stRecord;
                stRecord.UID = m_MoveRequest.UID;
                stRecord.AddTime = m_MoveRequest.AddTime;
                stRecord.X = m_MoveRequest.X;
                stRecord.Y = m_MoveRequest.Y;
                stRecord.R = m_MoveRequest.R;
                stRecord.PodAddress = (CharObject *)m_MoveRequest.Data->Activate();
                m_CharObjectRecordV.push_back(stRecord);
            }else{
                // just update
            }

        }

        m_MoveRequest.Clear();
    };

    m_ActorPod->Forward((bMoveOK ? MPK_OK : MPK_ERROR),
            m_MoveRequest.PodAddress, m_MoveRequest.MPKID, fnROP);
}

bool RegionMonitor::GroundValid(int, int, int)
{
    return true;
}


// object is moving inside a region, so there is no deleting / adding operation
// possible for message pack forwarding to neighbors
//
// Input : m_MoveRequest
// Output: always response to the requestor
//
void RegionMonitor::DoMoveAroundRequest()
{
    bool bMoveOK = false;
    // 1. check if the ground is OK for current moving request
    //    no collision check between objects here
    if(!GroundValid(m_MoveRequest.X, m_MoveRequest.Y, m_MoveRequest.R)){
        bMoveOK = false;
        goto __REGIONMONITOR_DOMOVEAROUNDREQUEST_DONE_1;
    }

    // 2. will I collide with any one in current region?
    for(auto &rstRecord: m_CharObjectRecordV){
        if(true
                && rstRecord.UID == m_MoveRequest.UID
                && rstRecord.AddTime == m_MoveRequest.AddTime){
            continue;
        }

        if(CircleOverlap(rstRecord.X, rstRecord.Y, rstRecord.R, 
                    m_MoveRequest.X, m_MoveRequest.Y, m_MoveRequest.R)){
            bMoveOK = false;
            goto __REGIONMONITOR_DOMOVEAROUNDREQUEST_DONE_1;
        }
    }

    // 4. check if I am only in current region?
    if(RectangleInside(m_X, m_Y, m_W, m_H,
                m_MoveRequest.X - m_MoveRequest.R,
                m_MoveRequest.Y - m_MoveRequest.R,
                2 * m_MoveRequest.R,
                2 * m_MoveRequest.R)){
        // yes only in current region
        // all-set
        bMoveOK = true;
        goto __REGIONMONITOR_DOMOVEAROUNDREQUEST_DONE_1;
    }

    // 5. OK I am in current region and cover some neighbor-regions
    //    need to check neighbors
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            if(nX == 1 && nY == 1){
                m_NeighborV2D[nY][nX].Query = 1;
                continue;
            }

            int nNeighborX = ((int)nX - 1) * m_W + m_X;
            int nNeighborY = ((int)nY - 1) * m_H + m_Y;

            if(RectangleOverlap(nNeighborX, nNeighborY, m_W, m_H, m_X, m_Y, m_W, m_H)){
                if(!m_NeighborV2D[nY][nX].Valid()){
                    // ok this neighbor is not valid to hold any objects
                    bMoveOK = false;
                    goto __REGIONMONITOR_DOMOVEAROUNDREQUEST_DONE_1;
                }

                // we need to ask for opinion from this neighbor
                m_NeighborV2D[nY][nX].Query = -1;
                auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                    m_NeighborV2D[nY][nX].Query = (rstRMPK.Type() == MPK_OK ? 1 : 0);
                };

                AMTryMove stAMTM;
                std::memcpy(&stAMTM, 0, sizeof(stAMTM));

                stAMTM.X       = m_MoveRequest.X;
                stAMTM.Y       = m_MoveRequest.Y;
                stAMTM.R       = m_MoveRequest.R;
                stAMTM.UID     = m_MoveRequest.UID;
                stAMTM.AddTime = m_MoveRequest.AddTime;

                m_ActorPod->Forward({MPK_TRYMOVE, stAMTM}, m_NeighborV2D[nY][nX].PodAddress, fnROP);
            }else{
                // has nothing to do with this neighbor, just set as OK
                m_NeighborV2D[nY][nX].Query = 1;
            }
        }
    }

    // 6. I have sent query to all my qualified neighbors, I'm done
    return;

__REGIONMONITOR_DOMOVEAROUNDREQUEST_DONE_1:
    auto fnROP = [this, bMoveOK](const MessagePack &rstRMPK, const Theron::Address &){
        // TODO & TBD
        // do I need to put this logic out of fnROP?
        // I give you the permission to move, and you take it
        if(bMoveOK && rstRMPK.Type() == MPK_OK){
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
        // TODO & TBD
        // do I need to broadcast the new location?
        // current the object queries locations of its neighbor when needed
        m_MoveRequest.Clear();
    };

    // it's a move around request, and stop here, so we are responding to
    // the object who requests this move
    m_ActorPod->Forward((bMoveOK ? MPK_OK : MPK_ERROR),
            m_MoveRequest.PodAddress, m_MoveRequest.MPKID, fnROP);
}

// object is coming in current region, maybe new object or space moving
// object, then we need to make decision whether we need to activate it
//
// Input : m_MoveRequest
// Output: always respond
//
void RegionMonitor::DoMoveInRequest()
{
    bool bMoveOK = false;
    // 1. check if the ground is OK for current moving request
    //    no collision check between objects here
    if(!GroundValid(m_MoveRequest.X, m_MoveRequest.Y, m_MoveRequest.R)){
        bMoveOK = false;
        goto __REGIONMONITOR_DOMOVEINREQUEST_DONE_1;
    }

    // 2. will I collide with any one in current region? don't have to 
    //    check ``same object" problem since it's moving in request
    for(auto &rstRecord: m_CharObjectRecordV){
        if(CircleOverlap(rstRecord.X, rstRecord.Y, rstRecord.R, 
                    m_MoveRequest.X, m_MoveRequest.Y, m_MoveRequest.R)){
            bMoveOK = false;
            goto __REGIONMONITOR_DOMOVEINREQUEST_DONE_1;
        }
    }

    // 4. I am in current region
    //    check if I am only in current region?
    if(RectangleInside(m_X, m_Y, m_W, m_H,
                m_MoveRequest.X - m_MoveRequest.R,
                m_MoveRequest.Y - m_MoveRequest.R,
                2 * m_MoveRequest.R,
                2 * m_MoveRequest.R)){
        // yes only in current region
        // all-set
        bMoveOK = true;
        goto __REGIONMONITOR_DOMOVEINREQUEST_DONE_1;
    }

    // 5. OK I am in current region and cover some neighbor-regions
    //    need to check neighbors
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            if(nX == 1 && nY == 1){
                m_NeighborV2D[nY][nX].Query = 1;
                continue;
            }

            int nNeighborX = ((int)nX - 1) * m_W + m_X;
            int nNeighborY = ((int)nY - 1) * m_H + m_Y;

            if(RectangleOverlap(nNeighborX, nNeighborY, m_W, m_H, m_X, m_Y, m_W, m_H)){
                if(!m_NeighborV2D[nY][nX].Valid()){
                    // ok this neighbor is not valid to hold any objects
                    bMoveOK = false;
                    goto __REGIONMONITOR_DOMOVEINREQUEST_DONE_1;
                }

                // we need to ask for opinion from this neighbor
                m_NeighborV2D[nY][nX].Query = -1;
                auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                    m_NeighborV2D[nY][nX].Query = (rstRMPK.Type() == MPK_OK ? 1 : 0);
                };

                AMTryMove stAMTM;
                stAMTM.X       = m_MoveRequest.X;
                stAMTM.Y       = m_MoveRequest.Y;
                stAMTM.R       = m_MoveRequest.R;
                stAMTM.UID     = m_MoveRequest.UID;
                stAMTM.AddTime = m_MoveRequest.AddTime;

                m_ActorPod->Forward({MPK_MOVE, stAMTM}, m_NeighborV2D[nY][nX].PodAddress, fnROP);
            }else{
                m_NeighborV2D[nY][nX].Query = 1;
            }
        }
    }

    // 6. I have sent query to all my qualified neighbors, I'm done
    return;

__REGIONMONITOR_DOMOVEREQUEST_DONE_1:
    auto fnROP = [this](const MessagePack &, const Theron::Address &rstFromAddr){
        // TODO & TBD
        // do I need to put this logic out of fnROP?
        if(bMoveOK && m_MoveRequest.In){
            if(stRecord.Data){
                // this is a new object
                CharObjectRecord stRecord;
                stRecord.UID = m_MoveRequest.UID;
                stRecord.AddTime = m_MoveRequest.AddTime;
                stRecord.X = m_MoveRequest.X;
                stRecord.Y = m_MoveRequest.Y;
                stRecord.R = m_MoveRequest.R;
                stRecord.PodAddress = (CharObject *)m_MoveRequest.Data->Activate();
                m_CharObjectRecordV.push_back(stRecord);
            }else{
                // just update
            }

        }

        m_MoveRequest.Clear();
    };

    m_ActorPod->Forward((bMoveOK ? MPK_OK : MPK_ERROR),
            m_MoveRequest.PodAddress, m_MoveRequest.MPKID, fnROP);




}

void RegionMonitor::DoMoveOutRequest()
{
    // it's from ``x" to 0 ~ 7 grid, local move, we don't need to
    // bother ServerMap for this move
    // +---+---+---+
    // | 0 | 1 | 2 |
    // +---+---+---+
    // | 3 | x | 4 |
    // +---+---+---+
    // | 5 | 6 | 7 |
    // +---+---+---+
    bool bLocalMove = PointInRectangle(
            m_MoveRequest.X, m_MoveRequest.Y, m_X - m_W, m_Y - m_H, 3 * m_W, 3 * m_H);
    Theron::Address stAddress = Theron::Address::Null();

    if(bLocalMove){
        int nX = (m_MoveRequest.X - (m_X - m_W)) / m_W;
        int nY = (m_MoveRequest.Y - (m_Y - m_H)) / m_H;

        if(nX < 0 || nX >= 3){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Logic error occurs");
            g_MonoServer->Restart();
        }

        stAddress = m_NeighborV2D[nY][nX].PodAddress;
    }else{
        stAddress = m_MapAddress;
    }

    std::string szObjectAddress = m_MoveRequest.PodAddress.GetAddress().AsString();

    AMTryMove stAMTM;
    stAMTM.X = m_MoveRequest.X;
    stAMTM.Y = m_MoveRequest.Y;
    stAMTM.DataLen = szObjectAddress.size();

    std::vector<uint8_t> stBuf.resize(sizeof(stAMTM) + szObjectAddress.size());
    std::memcpy(&stBuf[0], &stAMTM, sizeof(stAMTM));
    std::memcpy(&stBuf[sizeof(stAMTM)], szObjectAddress.c_str(), szObjectAddress.size());

    auto fnROP = [this](MessagePack &rstRMPK, Theron::Address &){
        if(rstRMPK.Type() == MPK_OK){
            for(auto pRecord = m_CharObjectRecordV.begin();
                    pRecord != m_CharObjectRecordV.end(), ++pRecord){
                if(pRecord->UID == m_MoveRequest.UID && pRecord->AddTime == m_MoveRequest.AddTime){
                    m_CharObjectRecordV.erase(pRecord);
                    return;
                }
            }
        }
    };
    m_ActorPod->Forward({MPK_TRYMOVE, stAMTM}, stAddress, m_MoveRequest.MPKID, fnROP);
}




// the object is added to current region from an un-continguous region
void RegionMonitor::DoSpaceMoveRequest()
{
}

void RegionMonitor::DoMoveRequest()
{
    bool bMoveOK = false;
    // 1. check if the ground is OK for current moving request
    //    no collision check between objects here
    if(m_MoveRequest.In){
        if(!GroundValid(m_MoveRequest.X, m_MoveRequest.Y, m_MoveRequest.R)){
            bMoveOK = false;
            goto __REGIONMONITOR_DOMOVEREQUEST_DONE_1;
        }
    }

    // 2. will I collide with any one in current region?
    if(m_MoveRequest.In){
        for(auto &rstRecord: m_CharObjectRecordV){
            if(!m_MoveRequest.Data){
                // this is a moving for an existing object
                //
                // no matter the region that the monster is in, or
                // neighbor region, this check is always OK, since
                // (UID, AddTime) is unique
                if(true
                        && rstRecord.UID == m_MoveRequest.UID
                        && rstRecord.AddTime == m_MoveRequest.AddTime){
                    continue;
                }
            }

            if(CircleOverlap(rstRecord.X, rstRecord.Y, rstRecord.R, 
                        m_MoveRequest.X, m_MoveRequest.Y, m_MoveRequest.R)){
                bMoveOK = false;
                goto __REGIONMONITOR_DOMOVEREQUEST_DONE_1;
            }
        }
    }

    // 3. is this a moving request by broadcast?
    //    we need to test this moving request happens in current region
    //    since region monitor will broadcast moving request among its neighbors
    if(!m_MoveRequest.In){
        // via broadcast, OK we are all-set
        // reply to the regioin hoding the moving object
        bMoveOK = true;
        goto __REGIONMONITOR_DOMOVEREQUEST_DONE_1;
    }

    // 4. I am in current region
    //    check if I am only in current region?
    if(RectangleInside(m_X, m_Y, m_W, m_H,
                m_MoveRequest.X - m_MoveRequest.R,
                m_MoveRequest.Y - m_MoveRequest.R,
                2 * m_MoveRequest.R,
                2 * m_MoveRequest.R)){
        // yes only in current region
        // all-set
        bMoveOK = true;
        goto __REGIONMONITOR_DOMOVEREQUEST_DONE_1;
    }

    // 5. OK I am in current region and cover some neighbor-regions
    //    need to check neighbors
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            if(nX == 1 && nY == 1){
                m_NeighborV2D[nY][nX].Query = 1;
                continue;
            }

            int nNeighborX = ((int)nX - 1) * m_W + m_X;
            int nNeighborY = ((int)nY - 1) * m_H + m_Y;

            if(RectangleOverlap(nNeighborX, nNeighborY, m_W, m_H, m_X, m_Y, m_W, m_H)){
                if(!m_NeighborV2D[nY][nX].Valid()){
                    // ok this neighbor is not valid to hold any objects
                    bMoveOK = false;
                    goto __REGIONMONITOR_DOMOVEREQUEST_DONE_1;
                }

                // we need to ask for opinion from this neighbor
                m_NeighborV2D[nY][nX].Query = -1;
                auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                    m_NeighborV2D[nY][nX].Query = (rstRMPK.Type() == MPK_OK ? 1 : 0);
                };

                AMTryMove stAMTM;
                stAMTM.X       = m_MoveRequest.X;
                stAMTM.Y       = m_MoveRequest.Y;
                stAMTM.R       = m_MoveRequest.R;
                stAMTM.UID     = m_MoveRequest.UID;
                stAMTM.AddTime = m_MoveRequest.AddTime;

                m_ActorPod->Forward({MPK_MOVE, stAMTM}, m_NeighborV2D[nY][nX].PodAddress, fnROP);
            }else{
                m_NeighborV2D[nY][nX].Query = 1;
            }
        }
    }

    // 6. I have sent query to all my qualified neighbors, I'm done
    return;

__REGIONMONITOR_DOMOVEREQUEST_DONE_1:
    auto fnROP = [this](const MessagePack &, const Theron::Address &rstFromAddr){
        // TODO & TBD
        // do I need to put this logic out of fnROP?
        if(bMoveOK && m_MoveRequest.In){
            if(stRecord.Data){
                // this is a new object
                CharObjectRecord stRecord;
                stRecord.UID = m_MoveRequest.UID;
                stRecord.AddTime = m_MoveRequest.AddTime;
                stRecord.X = m_MoveRequest.X;
                stRecord.Y = m_MoveRequest.Y;
                stRecord.R = m_MoveRequest.R;
                stRecord.PodAddress = (CharObject *)m_MoveRequest.Data->Activate();
                m_CharObjectRecordV.push_back(stRecord);
            }else{
                // just update
            }

        }

        m_MoveRequest.Clear();
    };

    m_ActorPod->Forward((bMoveOK ? MPK_OK : MPK_ERROR),
            m_MoveRequest.PodAddress, m_MoveRequest.MPKID, fnROP);
}
