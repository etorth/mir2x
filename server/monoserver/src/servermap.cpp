/*
 * =====================================================================================
 *
 *       Filename: servermap.cpp
 *        Created: 04/06/2016 08:52:57 PM
 *  Last Modified: 04/10/2016 02:04:25
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

#include "servermap.hpp"

bool ServerMap::Load(const char szMapFullName)
{
    if(!m_Mir2xMap.Load(szMapFullName)){ return false; }
    int nW = m_Mir2xMap.W();
    int nH = m_Mir2xMap.H();

    mGridObjectList.clear();
    mGridObjectList.resize(nH);
    for(auto &stLine: mGridObjectList){
        stLine.resize(nW);
    }
}

// test whether we can make a room for (nUID, nAddTime) at (nX, nY)
// assumption:
//      1. cell locked
//      2. there is only one object (nUID, nAddTime) locked
bool CanMove(int nX, int nY, int nR, uint32_t nUID, uint32_t nAddTime)
{
    // 1. always check argument
    if(false
            || !(nR > 0)
            || !m_Mir2xMap.Valid()
            || !m_Mir2xMap.ValidP(nX, nY)){
        return false;
    }

    int nCenterX  = nX / 48;
    int nCenterY  = nY / 32;
    int nGridSize = nR / 32 + 1; // to make it safe

    RotateCoord stRotateCoord;
    stRotateCoord.Reset(nCenterX, nCenterY, nCenterX, nCenterY, nGridSize);

    do{
        int nGridX = stRotateCoord.X();
        int nGridY = stRotateCoord.Y();

        if(!m_Mir2xMap.ValidC(nGridX, nGridY) || !m_Mir2xMap.CanWalk(nGridX, nGridY)){
            continue;
        }

        auto stInst = m_GridObjectRecordList[nGridX][nGridY].begin();
        while(stInst != m_GridObjectRecordList.end()){
            if(stInst->Type != OT_CHAROBJECT 
                    || (stInst->UID == nUID && stInst->AddTime == nAddTime)){
                continue; 
            }

            extern MonoServer *g_MonoServer;
            auto pGuard = g_MonoServer->CheckOut<CharObject>(stInst->UID, stInst->AddTime);

            if(!pGuard){
                stInst = m_GridObjectRecordList[nGridX][nGridY].erase(stInst);
                continue;
            }

            // now guard is valid
            if(true
                    && !pGuard->Dead()
                    && !pGuard->Inspector()
                    && !pGuard->Hide()){
                if(CircleOverlap(nX, nY, nR, pGuard->X() , pGuard->Y(), pGuard->R())){
                    return false;
                }
            }
        }
    }while(stRotateCoord.Forward());

    return true;
}

// move an object to a new position
// assumption
//      1. atomic
//      2. if failed, nothing changed
//      3. if succeed, position of pObject will be updated
//      4. pObject is valid for sure
bool ServerMap::ObjectMove(int nTargetX, int nTargetY, CharObject *pObject)
{
    if(!m_Mir2xMap.ValidP(nTargetX, nTargetY) || !pObject){
        return false;
    }

    int nGridX0 = pObject->X() / 48;
    int nGridY0 = pObject->Y() / 32;
    int nGridX1 = nTargetX / 48;
    int nGridY1 = nTargetY / 32;

    if(nGridX0 == nGridX1 && nGridY0 == nGridY1){
        std::lock_guard<std::mutex> stLockGuard(*m_GridObjectRecordListLock[nGridX0][nGridY0]);
        if(!CanMove(nTargetX, nTargetY, pObject->R(), pObject->UID(), pObject->AddTime())){
            return false;
        }

        pObject->Move(nTargetX, nTargetY);
    }else{
        std::lock_guard<std::mutex> stLockGuard1(*m_GridObjectRecordListLock[nGridX0][nGridY0]);
        std::lock_guard<std::mutex> stLockGuard2(*m_GridObjectRecordListLock[nGridX1][nGridY1]);

        if(!CanMove(nTargetX, nTargetY, pObject->R(), pObject->UID(), pObject->AddTime())){
            return false;
        }

        if(RemoveObject(nGridX0, nGridY0, OT_CHAROBJECT, pObject->UID(), pObject->AddTime())){
            if(AddObject(nGridX1, nGridY1, OT_CHAROBJECT, pObject->UID(), pObject->AddTime())){
                return true;
            }
            AddObject(nGridX0, nGridY0, OT_CHAROBJECT, pObject->UID(), pObject->AddTime());
        }
        return false;
    }
}

// assumption:
//      1. cell locked
//      2. new object is already present in the object hub
//      3. won't exam the validation for the new object
bool ServerMap::AddObject(int nGridX, int nGridY,
        uint8_t btType, uint32_t nUID, uint32_t nAddTime)
{
    if(!m_Mir2xMap.ValidP(nX, nY) || !m_Mir2xMap.CanWalk(nGridX, nGridY)){
        return false;
    }
    m_GridObjectRecordList[nGridY][nGridX].emplace_front(nType, pNewObject, nAddTime);
    return true;
}

// assumption:
//      1. cell locked
bool ServerMap::GetObjectList(int nGridX, int nGridY,
        std::forward_list<OBJECTRECORD> *pList, std::function<bool(uint8_t nType)> fnFindByType)
{
    if(!m_Mir2xMa.ValidC(nGridX, nGridY)){
        return false;
    }

    auto fnFind = [pList, &fnFindByType](const OBJECTRECORD &rstRecord){
        return fnFindByType(rstRecord.Type);
    }

    std::for_each(
            m_GridObjectRecordList[nGridX][nGridY].begin(),
            m_GridObjectRecordList[nGridX][nGridY].end(), fnFind);
    return true;
}

// assumption:
//      1. cell locked
bool ServerMap::CanSafeWalk(int nGridX, int nGridY)
{
    if(!m_Mir2xMap.validC(nGridX, nGridY) || !m_Mir2xMap.CanWalk(nGridX, nGridY)){
        return false;
    }

    auto fnFind = [](const OBJECTRECORD &rstRecord){
        if(rstRecord.Type == OT_EVENTOBJECT){
            extern MonoServer *g_MonoServer;
            auto pGuard = g_MonoServer->
                CheckOut<EventObject>(rstRecord.UID, rstRecord.AddTime);
            if(pGuard && pGuard->Damage() > 0){
                return true;
            }
        }
    };


    auto pFindInst = std::find_if(
            m_GridObjectRecordList[nGridX][nGridY].begin(),
            m_GridObjectRecordList[nGridX][nGridY].end(), fnFind);

    if(pFindInst != m_GridObjectRecordList[nGridY][nGridX].end()){
        return false;
    }

    return true;
}

// input may be invalid
// but this function guarantees no (nUID, nAddTime) after invocation
//
// assumption
//      1. cell locked, no else can update it asynchronously
//      2. won't check validation of object
bool ServerMap::RemoveObject(
        int nGridX, int nGridY, uint8_t nType, uint32_t nUID, uint32_t nAddTime)
{
    if(false
            || !m_Mir2xMap.ValidC(nGridX, nGridY)
            || !m_Mir2xMap.CanWalk(nGridX, nGridY)
            || !m_GridObjectRecordList.empty()){
        return false;
    }

    auto fnCmp = [nType, nUID, nAddTime](const OBJECTRECORD &rscRecord){
        return true
            && rstRecord.Type    == nType
            && rstRecord.UID     == nUID
            && rstRecord.AddTime == nAddTime;
    };

    std::erase(std::remove_if(
                m_GridObjectRecordList[nGridX][nGridY].begin(),
                m_GridObjectRecordList[nGridX][nGridY].end(), fnCmp));
}
