/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.cpp
 *        Created: 04/22/2016 01:15:24
 *  Last Modified: 05/04/2016 22:52:11
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
// when got MPK_ADDMONSTOR, MPK_ADDPLAYER, or MPK_TRYMOVE in the
// region monitor, monitor will check whether the corresponding object
// is in current region, if not, it just check object collosion and
// ground validation, and respond the ``main" region ERROR or OK
//
// if it's in current region, it checks object collision, ground
// validation, and broadcast this moving request to all its qualified 
// neighbors.
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
    if(!PointInRectangle(m_MoveRequest.X, m_MoveRequest.Y, m_X, m_Y, m_W, m_H)){
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
    auto fnROP = [this](const MessagePack &, const Theron::Address &){
        m_MoveRequest.Clear();
    };
    m_ActorPod->Forward((bMoveOK ? MPK_OK : MPK_ERROR),
            m_MoveRequest.PodAddress, m_MoveRequest.MPKID, fnROP);
}

bool RegionMonitor::GroundValid(int, int, int)
{
    return true;
}
