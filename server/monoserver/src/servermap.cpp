/*
 * =====================================================================================
 *
 *       Filename: servermap.cpp
 *        Created: 04/06/2016 08:52:57 PM
 *  Last Modified: 04/11/2016 22:50:35
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

#include <algorithm>

#include "mathfunc.hpp"
#include "sysconst.hpp"
#include "servermap.hpp"
#include "charobject.hpp"
#include "monoserver.hpp"
#include "rotatecoord.hpp"

bool ServerMap::Load(const char *szMapFullName)
{
    if(!m_Mir2xMap.Load(szMapFullName)){ return false; }
    int nW = m_Mir2xMap.W();
    int nH = m_Mir2xMap.H();

    m_GridObjectRecordListV.clear();
    m_GridObjectRecordListLockV.clear();

    m_GridObjectRecordListV.resize(nH);
    for(auto &stLine: m_GridObjectRecordListV){ stLine.resize(nW); }

    m_GridObjectRecordListLockV.resize(nH);
    for(auto &stLine: m_GridObjectRecordListLockV){
        // TODO do we have an emplace_back() with count?
        stLine.insert(stLine.begin(), nW, std::make_shared<std::mutex>());
    }

    return true;
}

// // test whether we can make a room for (nUID, nAddTime) at (nX, nY)
// // assumption:
// //      1. cell locked
// //      2. there is only one object (nUID, nAddTime) locked
// bool ServerMap::CanMove(int nX, int nY, int nR, uint32_t nUID, uint32_t nAddTime)
// {
//     // 1. always check argument
//     if(false
//             || !(nR > 0)
//             || !m_Mir2xMap.Valid()
//             || !m_Mir2xMap.ValidP(nX, nY)){
//         return false;
//     }
//
//     int nCenterX  = nX / 48;
//     int nCenterY  = nY / 32;
//     int nGridSize = nR / 32 + 1; // to make it safe
//
//     RotateCoord stRotateCoord;
//     stRotateCoord.Reset(nCenterX, nCenterY, nCenterX, nCenterY, nGridSize, nGridSize);
//
//     do{
//         int nGridX = stRotateCoord.X();
//         int nGridY = stRotateCoord.Y();
//
//         if(!m_Mir2xMap.ValidC(nGridX, nGridY) || !m_Mir2xMap.CanWalk(nGridX, nGridY, nR)){
//             continue;
//         }
//
//         auto stInst = m_GridObjectRecordListV[nGridY][nGridX].begin();
//         while(stInst != m_GridObjectRecordListV[nGridY][nGridX].end()){
//             if(stInst->Type != OT_CHAROBJECT 
//                     || (stInst->UID == nUID && stInst->AddTime == nAddTime)){
//                 continue; 
//             }
//
//             extern MonoServer *g_MonoServer;
//             auto pGuard = g_MonoServer->CheckOut<CharObject>(stInst->UID, stInst->AddTime);
//
//             if(!pGuard){
//                 stInst = m_GridObjectRecordListV[nGridY][nGridX].erase(stInst);
//                 continue;
//             }
//
//             // now guard is valid
//             if(true
//                     && !pGuard->Dead()
//                     && !pGuard->Inspector()
//                     && !pGuard->Hide()){
//                 if(CircleOverlap(nX, nY, nR, pGuard->X() , pGuard->Y(), pGuard->R())){
//                     return false;
//                 }
//             }
//         }
//     }while(stRotateCoord.Forward());
//
//     return true;
// }

// move an object to a new position, it's in a try-fail manner, if succeed everyting
// takes place, if not return false and nothing changes
// assumption
//      1. atomic
//      2. if failed, nothing changed
//      3. if succeed, position of pObject will be updated
//      4. pObject is valid for sure
bool ServerMap::ObjectMove(int nTargetX, int nTargetY, CharObject *pObject)
{
    // this funciton require 100% logic correctness
    // have to lock an area to prevent any update inside
    if(!m_Mir2xMap.ValidP(nTargetX, nTargetY) || !pObject){
        return false;
    }

    // need to use system defined max R for safety
    int nMaxLD = pObject->R() + SYS_MAXR;

    int nGridX0 = (nTargetX - nMaxLD) / SYS_GRIDXP;
    int nGridX1 = (nTargetX + nMaxLD) / SYS_GRIDXP;
    int nGridY0 = (nTargetY - nMaxLD) / SYS_GRIDYP;
    int nGridY1 = (nTargetY + nMaxLD) / SYS_GRIDYP;

    int nGridW = nGridX1 - nGridX0 + 1;
    int nGridH = nGridY1 - nGridY0 + 1;

    int nGridXS = pObject->X() / SYS_GRIDXP;
    int nGridYS = pObject->Y() / SYS_GRIDYP;

    bool bLockS = LockArea(true, nGridXS, nGridYS, 1, 1);
    bool bLockD = LockArea(true, nGridX0, nGridY0, nGridW, nGridH, nGridXS, nGridYS);

    // only work for both s/c locked
    if(!(bLockS && bLockD)){
        if(bLockS){ LockArea(false, nGridXS, nGridYS, 1, 1); }
        if(bLockD){ LockArea(false, nGridX0, nGridY0, nGridW, nGridH, nGridXS, nGridYS); }
        return false;
    }

    // now srd/dst are both locked
    for(int nY = nGridY0; nY <= nGridY1; ++nY){
        for(int nX = nGridX0; nX <= nGridX1; ++nX){
            auto pRecord = m_GridObjectRecordListV[nY][nX].begin();
            while(pRecord != m_GridObjectRecordListV[nY][nX].end()){
                // for item/event object, we won't validate it
                if(std::get<0>(*pRecord) != OBJECT_CHAROBJECT){
                    pRecord++;
                    continue;
                }

                // skip self
                if(std::get<1>(*pRecord) == pObject->UID()
                        && std::get<2>(*pRecord) == pObject->AddTime()){
                    pRecord++;
                    continue;
                }
                // else its an valid charobject candidate
                // 1. validate it
                // 2. check distance
                extern MonoServer *g_MonoServer;
                if(auto pGuard = g_MonoServer->CheckOut<CharObject>(
                            std::get<1>(*pRecord), std::get<2>(*pRecord))){
                    // this record is still available
                    if(CircleOverlap(
                                pGuard->X(), pGuard->Y(), pGuard->R(),
                                pObject->X(), pObject->Y(), pObject->R())){

                        LockArea(false, nGridXS, nGridYS, 1, 1);
                        LockArea(false, nGridX0, nGridY0, nGridW, nGridH, nGridXS, nGridYS);
                        return false;
                    }
                }else{
                    pRecord = m_GridObjectRecordListV[nY][nX].erase(pRecord);
                }
            }
        }
    }

    // test all and didn't find any collision
    // 1. remove from previous list
    ObjectRecord stObjectRecord {OBJECT_CHAROBJECT, pObject->UID(), pObject->AddTime()};
    m_GridObjectRecordListV[nGridYS][nGridXS].erase(std::remove(
                m_GridObjectRecordListV[nGridYS][nGridXS].begin(),
                m_GridObjectRecordListV[nGridYS][nGridXS].end(), stObjectRecord));
                // {OBJECT_CHAROBJECT, pObject->UID(), pObject->AddTime()}));

    // 2. add to new cell
    m_GridObjectRecordListV[nTargetY / SYS_GRIDYP][nTargetX / SYS_GRIDXP].emplace_front(
            stObjectRecord);
            // {OBJECT_CHAROBJECT, pObject->UID(), pObject->AddTime()});

    // 3. Unlock all cells
    LockArea(false, nGridXS, nGridYS, 1, 1);
    LockArea(false, nGridX0, nGridY0, nGridW, nGridH, nGridXS, nGridYS);

    return true;
}

// assumption:
//      1. cell locked
//      2. new object is already present in the object hub
//      3. won't exam the validation for the new object
bool ServerMap::AddObject(int nGridX, int nGridY,
        uint8_t nType, uint32_t nUID, uint32_t nAddTime)
{
    if(!m_Mir2xMap.ValidC(nGridX, nGridY) || !m_Mir2xMap.CanWalk(nGridX, nGridY)){
        return false;
    }
    m_GridObjectRecordListV[nGridY][nGridX].emplace_front(nType, nUID, nAddTime);
    return true;
}

// // assumption:
// //      1. cell locked
// bool ServerMap::GetObjectList(int nGridX, int nGridY,
//         std::forward_list<OBJECTRECORD> *pList, std::function<bool(uint8_t nType)> fnFindByType)
// {
//     if(!m_Mir2xMa.ValidC(nGridX, nGridY)){
//         return false;
//     }
//
//     auto fnFind = [pList, &fnFindByType](const OBJECTRECORD &rstRecord){
//         return fnFindByType(rstRecord.Type);
//     }
//
//     std::for_each(
//             m_GridObjectRecordListV[nGridX][nGridY].begin(),
//             m_GridObjectRecordListV[nGridX][nGridY].end(), fnFind);
//     return true;
// }

// // assumption:
// //      1. cell locked
// bool ServerMap::CanSafeWalk(int nGridX, int nGridY)
// {
//     if(!m_Mir2xMap.ValidC(nGridX, nGridY) || !m_Mir2xMap.CanWalk(nGridX, nGridY)){
//         return false;
//     }
//
//     auto fnFind = [](const OBJECTRECORD &rstRecord){
//         if(rstRecord.Type == OT_EVENTOBJECT){
//             extern MonoServer *g_MonoServer;
//             auto pGuard = g_MonoServer->
//                 CheckOut<EventObject>(rstRecord.UID, rstRecord.AddTime);
//             if(pGuard && pGuard->Damage() > 0){
//                 return true;
//             }
//         }
//     };
//
//
//     auto pFindInst = std::find_if(
//             m_GridObjectRecordListV[nGridX][nGridY].begin(),
//             m_GridObjectRecordListV[nGridX][nGridY].end(), fnFind);
//
//     if(pFindInst != m_GridObjectRecordListV[nGridY][nGridX].end()){
//         return false;
//     }
//
//     return true;
// }

// // input may be invalid
// // but this function guarantees no (nUID, nAddTime) after invocation
// //
// // assumption
// //      1. cell locked, no else can update it asynchronously
// //      2. won't check validation of object
// bool ServerMap::RemoveObject(
//         int nGridX, int nGridY, uint8_t nType, uint32_t nUID, uint32_t nAddTime)
// {
//     if(false
//             || !m_Mir2xMap.ValidC(nGridX, nGridY)
//             || !m_Mir2xMap.CanWalk(nGridX, nGridY)
//             || !m_GridObjectRecordListV.empty()){
//         return false;
//     }
//
//     auto fnCmp = [nType, nUID, nAddTime](const OBJECTRECORD &rscRecord){
//         return true
//             && rstRecord.Type    == nType
//             && rstRecord.UID     == nUID
//             && rstRecord.AddTime == nAddTime;
//     };
//
//     m_GridObjectRecordListV[nGridX][nGridY].erase(
//             std::remove_if(
//                 m_GridObjectRecordListV[nGridX][nGridY].begin(),
//                 m_GridObjectRecordListV[nGridX][nGridY].end(), fnCmp));
// }

// lock the gird (i, j) and invoke the handler for each object
// in this function m_GridObjectRecordListV is update by in/out event driven
// every time there is an object being inside/outside of a grid, the w.r.t.
// gird will be updated, so (TODO) fnQuery can assume each each object it
// queried is still in the grid
//
// even fnQuery found current object is not in current grid, it can't update
// the gird list directly since it only got an ID of the object. update done
// by event driven so it's OK
//
bool ServerMap::QueryObject(int nX, int nY,
        const std::function<void(uint8_t, uint32_t, uint32_t)> &fnQuery)
{
    // 1. check arguments
    if(!m_Mir2xMap.ValidC(nX, nY)){ return false; }

    {
        // 2. lock cell (i, j)
        std::lock_guard<std::mutex> stLockGuard(*m_GridObjectRecordListLockV[nY][nX]);

        // 3. invoke the handler
        for(auto &pRecord: m_GridObjectRecordListV[nY][nX]){
            fnQuery(std::get<0>(pRecord), std::get<1>(pRecord), std::get<0>(pRecord));
        }
    }

    return true;
}
