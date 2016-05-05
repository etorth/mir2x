/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.cpp
 *        Created: 04/22/2016 01:15:24
 *  Last Modified: 05/04/2016 19:04:30
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

#include "actorpod.hpp"
#include "regionmonitor.hpp"
#include "monoserver.hpp"

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
// Output:  none
//
// try to keep it simple, if someone else is moving, just reject this
// request, you can check the moving object and make sure this moving
// object won't cause collision, but that adds complexity
void RegionMonitor::DoMoveRequest()
{
    // 1. will I collide with any one in current region?
    for(auto &rstRecord: m_CharObjectRecordV){
        if(!m_MoveRequest.Data){
            // this is a moving for an existing object
            if(true
                    && rstRecord.UID == m_MoveRequest.UID
                    && rstRecord.AddTime == m_MoveRequest.AddTime){
                continue;
            }
        }

        if(CircleOverlap(stAMNM.X, stAMNM.Y, stAMNM.R, rstRecord.X, rstRecord.Y, rstRecord.R)){
            goto __REGIONMONITOR_ADDNEWMONSITER_FAIL_1;
        }
    }

    // 2. am I in current region only?
    if(RectangleInside(m_X, m_Y, m_W, m_H,
                stAMNM.X - stAMNM.R, stAMNM.Y - stAMNM.R, stAMNM.R * 2, stAMNM.R * 2)){
        // ok we are all set
        if(rstMPK.ID()){
            m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
        }
        return;
    }

    // 3. ooops we need to check neighbors
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            if(nX == 1 && nY == 1){
                m_NeighborV2D[nY][nX].Query = true;
                continue;
            }

            int nNeighborX = ((int)nX - 1) * m_W + m_X;
            int nNeighborY = ((int)nY - 1) * m_H + m_Y;

            if(RectangleOverlap(nNeighborX, nNeighborY, m_W, m_H, m_X, m_Y, m_W, m_H)){
                if(!m_NeighborV2D[nY][nX].Valid()){
                    // ok this neighbor is not valid to hold any objects
                    goto __REGIONMONITOR_ADDNEWMONSITER_FAIL_1;
                }

                // we need to ask for opinion from this neighbor
                m_NeighborV2D[nY][nX].Query = -1;
                auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                    m_NeighborV2D[nY][nX].Query = (rstRMPK.Type() == MPK_OK ? 1 : 0);
                };
                m_ActorPod->Forward(rstMPK, m_NeighborV2D[nY][nX].PodAddress, fnROP);
            }

            m_CharObjectAddressL.push_back(pNewMonster->Activate());
            m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());


        }
    }

__REGIONMONITOR_ADDNEWMONSITER_FAIL_1:
    if(rstMPK.ID()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
    }else{
        delete pNewMonster;
    }
}
