/*
 * =====================================================================================
 *
 *       Filename: charobject.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 06/05/2016 19:46:09
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
    , m_EmptyAddress(Theron::Address::Null())
    , m_RMAddress(Theron::Address::Null())
    , m_MapAddress(Theron::Address::Null())
    , m_SCAddress(Theron::Address::Null())
    , m_RMAddressQuery(QUERY_NA)
    , m_MapAddressQuery(QUERY_NA)
    , m_SCAddressQuery(QUERY_NA)
    , m_MapID(0)
    , m_CurrX(0)
    , m_CurrY(0)
    , m_R(0)
    , m_Direction(DIR_UP)
    , m_Name("")
{}

CharObject::~CharObject()
{}

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
    if(!ActorPodValid()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "motion dispatching require actor to be activated");
        g_MonoServer->Restart();
    }

    AMMotionState stAMMS;
    stAMMS.X = m_CurrX;
    stAMMS.Y = m_CurrY;

    // ok we have map address
    if(m_MapAddress){
        m_ActorPod->Forward({MPK_MOTIONSTATE, stAMMS}, m_MapAddress);
        return;
    }

    // no we don't have map address
    if(!m_MapID){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "logic error, activated charobject are with zero map id");
        g_MonoServer->Restart();
    }

    if(!m_RMAddress){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "activated char object with null RM address");
        g_MonoServer->Restart();
    }

    auto fnOnR = [this, stAMMS](const MessagePack &rstRMPK, const Theron::Address &){
        // 0. even failed, we won't repeat the request
        if(rstRMPK.Type() != MPK_ADDRESS){ return; }
        // 1. keep the record
        m_MapAddress = Theron::Address((const char *)(rstRMPK.Data()));
        // 2. dispatch
        m_ActorPod->Forward({MPK_MOTIONSTATE, stAMMS}, m_MapAddress);
    };
    m_ActorPod->Forward({MPK_QUERYMAPADDRESS, m_MapID}, m_RMAddress, fnOnR);
}
