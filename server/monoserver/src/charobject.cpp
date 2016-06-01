/*
 * =====================================================================================
 *
 *       Filename: charobject.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 05/31/2016 18:43:33
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
#include "monoserver.hpp"
#include "charobject.hpp"
#include "messagepack.hpp"
#include "eventtaskhub.hpp"

CharObject::CharObject()
    : ActiveObject()
    , m_R(0)
    , m_MapID(0)
    , m_CurrX(0)
    , m_CurrY(0)
    , m_Name("")
{
    m_StateAttrV.fill(false);
}


CharObject::~CharObject()
{
}

// void CharObject::DropItem(uint32_t nUID, uint32_t nAddTime, int nRange)
// {
//     int nMapID = m_Map->ID();
//
//     auto fnAddItem = [nMapID, nGUID, nID, nAddTime, nRange, m_CurrX, m_CurrY](){
//         extern MonoServer *g_MonoServer;
//         if(auto pMap = g_MonoServer->MapGrab(nMapID)){
//             int nX, nY;
//             if(pMap->DropLocation(m_CurrY, m_CurrY, nRange, &nX, &nY)){
//                 pMap->AddItem(nX, nY, nGUID, nID, nAddTime);
//             }
//         }
//     };
//
//     extern TaskHub *g_TaskHub;
//     g_TaskHub->Add(fnAddItem);
// }

void CharObject::Die()
{
    if(Mode(STATE_NEVERDIE)){ return; }
    ResetState(STATE_DEAD, true);
}

void CharObject::NextLocation(int *pX, int *pY, int nDistance)
{
    double fDX[] = {+0.000, +0.707, +1.000, +0.707, +0.000, -0.707, -1.000, -0.707};
    double fDY[] = {-1.000, -0.707, +0.000, +0.707, +1.000, +0.707, +0.000, -0.707};

    if(pX){ *pX = m_CurrX + std::lround(fDX[m_Direction] * nDistance); }
    if(pY){ *pY = m_CurrY + std::lround(fDY[m_Direction] * nDistance); }
}

uint8_t CharObject::Direction(int nX, int nY)
{
    int nDX = nX - m_CurrX;
    int nDY = nY - m_CurrY;

    uint8_t nDirection = 0;
    if(nDX == 0){
        if(nDY > 0){
            nDirection = 4;
        }else{
            nDirection = 0;
        }
    }else{
        double dATan = std::atan(1.0 * nDY / nDX);
        if(nDX > 0){
            nDirection = (uint8_t)(std::lround(2.0 + dATan * 4.0 / 3.1416) % 8);
        }else{
            nDirection = (uint8_t)(std::lround(6.0 + dATan * 4.0 / 3.1416) % 8);
        }
    }
    return nDirection;
}

void CharObject::DispatchMotion()
{
    if(!(m_ActorPod && m_ActorPod->GetAddress())){ return; }
    AMMotionState stAMMS;

    stAMMS.X = m_CurrX;
    stAMMS.Y = m_CurrY;

    if(m_MapAddress){
        m_ActorPod->Forward({MPK_MOTIONSTATE, stAMMS}, m_MapAddress);
        return;
    }

    if(m_MapID != 0){
        auto fnOnGetMapAddr = [this, stAMMS](const MessagePack & rstRMPK, const Theron::Address &){
            if(rstRMPK.Type() != MPK_ADDRESS){ return; }
            m_MapAddress = Theron::Address((const char *)(rstRMPK.Data()));

            m_ActorPod->Forward({MPK_MOTIONSTATE, stAMMS}, m_MapAddress);
        };
        m_ActorPod->Forward({MPK_QUERYMAPADDRESS, m_MapID}, m_SCAddress, fnOnGetMapAddr);
    }
}
