/*
 * =====================================================================================
 *
 *       Filename: charobject.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 04/14/2016 00:35:01
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

#include "monoserver.hpp"
#include "charobject.hpp"
#include "eventtaskhub.hpp"

CharObject::CharObject(uint32_t nUID, uint32_t nAddTime)
    : ActiveObject(nUID, nAddTime)
    , m_CurrX(0)
    , m_CurrY(0)
    , m_Event(-1)
    , m_Master(0, 0)
    , m_Target(0, 0)
    , m_Name("")
{
    extern MonoServer *g_MonoServer;
    m_AddTime = g_MonoServer->GetTickCount();
    m_StateAttrV.fill(0);
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
    SetState(STATE_DEAD, true);
}

// bool CharObject::Move()
// {
//     extern MonoServer *g_MonoServer;
//     uint32_t nNow = g_MonoServer->GetTickCount();
//
//     if(m_LastEvent == EVENT_MOVE){
//         uint32_t nDTick = nNow - m_LastEventTick;
//         int nDistance = Speed() * (int)nDTick;
//
//         int nX, nY;
//         NextLocation(&nX, &nY, nDistance);
//     }
//
// }

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

// bool CharObject::RangeTask(uint8_t nRangeType,
//         std::function<void(CharObjectID, CharObjectID)> fnOp)
// {
//     switch(nRangeType){
//         case RT_AROUND:
//             {
//                 if(m_CacheObjectList.empty()
//                         || g_MonoServer->GetTickCount() - m_CacheTick > SYS_CACHETICK){
//                     int nStartX = m_CurrX - SYS_RANGEX;
//                     int nStopX  = m_CurrX + SYS_RANGEX;
//                     int nStartY = m_CurrY - SYS_RANGEY;
//                     int nStopY  = m_CurrY + SYS_RANGEY;
//
//                     m_CacheObjectList.clear();
//                     for(int nX = nStartX; nX <= nStopX; ++nX){
//                         for(int nY = nStartY; nY <= nStopY; ++nY){
//                             std::forward_list<CharObjectID> stList;
//                             m_Map->GetObjectList(nX, nY, &stList);
//                             m_CacheObjectList.insert(
//                                     m_CacheObjectList.end(), stList.begin(), stList.end());
//                         }
//                     }
//                 }
//
//                 for(auto &stID: m_CacheObjectList){
//                     fnOp(m_ObjectID, stID);
//                 }
//
//                 return true;
//             }
//
//         case RT_MAP:
//             {
//                 return true;
//             }
//         case RT_SERVER:
//             {
//                 return true;
//             }
//         default:
//             {
//                 return false;
//             }
//     }
//
//     return false;
// }
