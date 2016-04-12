/*
 * =====================================================================================
 *
 *       Filename: charobject.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 04/11/2016 12:14:47
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

CharObject::CharObject()
    : m_CurrX(0)
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
    std::for_each(
            m_VisibleObjectList.first(),
            m_VisibleObjectList.end(),
            [](VisibleObject *pObject){ delete pObject; });
    m_VisibleObjectList.clear();
}

void CharObject::DropItem(uint32_t nUID, uint32_t nAddTime, int nRange)
{
    int nMapID = m_Map->ID();

    auto fnAddItem = [nMapID, nGUID, nID, nAddTime, nRange, m_CurrX, m_CurrY](){
        extern MonoServer *g_MonoServer;
        if(auto pMap = g_MonoServer->MapGrab(nMapID)){
            int nX, nY;
            if(pMap->DropLocation(m_CurrY, m_CurrY, nRange, &nX, &nY)){
                pMap->AddItem(nX, nY, nGUID, nID, nAddTime);
            }
        }
    };

    extern TaskHub *g_TaskHub;
    g_TaskHub->Add(fnAddItem);
}

void CharObject::Die()
{
    if(m_NeverDie){ return; }

    m_Dead = true;
    extern EventTaskHub *g_EventTaskHub;

    auto fnMakeGhost = [m_UID, m_AddTime](){
        extern MonoServer *g_MonoServer;
        if(auto pObject = g_MonoServer->CharObjectGrab(m_UID, m_AddTime)){
            pObject->Ghost(true);
            pObject->Unlock();
        }
    };
    g_EventTaskHub->Add(3 * 60 * 1000, fnMakeGhost);
}

bool CharObject::Locate()
{
    if(!State(STATE_MOVING)){ return false; }

    extern MonoServer *g_MonoServer;
    int nDTime    = g_MonoServer->GetTickCount() - m_LastMoveTime;
    int nDistance = std::max(1, nDTime * Speed());

    if(m_Map->CanMove()){
    }
}

int CharObject::GetNextDirection(int nStartX, int nStartY, int nTargetX, int nTargetY)
{
    int nFlagX, nFlagY;

    if(nStartX < nTargetX) nFlagX = 1;
    else if(nStartX == nTargetX) nFlagX = 0;
    else
        nFlagX = -1;

    if(abs(nStartY - nTargetY) > 2)
    {
        if((nStartY >= nTargetY - 1) && (nStartY <= nTargetY + 1))
            nFlagX = 0;
    }

    if(nStartY < nTargetY) nFlagY = 1;
    else if(nStartY == nTargetY) nFlagY = 0;
    else
        nFlagY = -1;

    if(abs(nStartX - nTargetX) > 2)
    {
        if((nStartX >= nTargetX - 1) && (nStartX <= nTargetX + 1))
            nFlagY = 0;
    }

    if((nFlagX == 0) && (nFlagY == -1)) return DR_UP;
    else if((nFlagX == 1) && (nFlagY == -1)) return DR_UPRIGHT;
    else if((nFlagX == 1) && (nFlagY == 0)) return DR_RIGHT;
    else if((nFlagX == 1) && (nFlagY == 1)) return DR_DOWNRIGHT;
    else if((nFlagX == 0) && (nFlagY == 1)) return DR_DOWN;
    else if((nFlagX == -1) && (nFlagY == 1)) return DR_DOWNLEFT;
    else if((nFlagX == -1) && (nFlagY == 0)) return DR_LEFT;
    else if((nFlagX == -1) && (nFlagY == -1)) return DR_UPLEFT;

    return DR_DOWN;
}

CharObject* CharObject::GetFrontObject()
{
    int nX, nY;

    GetFrontPosition(nX, nY);

    CharObject* pCharObject = m_Map->GetObject(nX, nY);

    if(pCharObject)
        return pCharObject;

    return NULL;
}

void CharObject::SpaceMove(int nX, int nY, CMirMap* pMirMap)
{
    CVisibleObject* pVisibleObject;

    if(m_Map->RemoveObject(m_CurrX, m_CurrY, OS_MOVINGOBJECT, this))
    {
        PLISTNODE pListNode = m_VisibleObjectList.GetHead();

        while (pListNode)
        {
            pVisibleObject = m_VisibleObjectList.GetData(pListNode);

            if(pVisibleObject)
            {
                delete pVisibleObject;
                pVisibleObject = NULL;
            }

            pListNode = m_VisibleObjectList.RemoveNode(pListNode);
        } // while (pListNode)

        m_Map = pMirMap;

        m_CurrX = nX;
        m_CurrY = nY;

        if(m_Map->AddNewObject(m_CurrX, m_CurrY, OS_MOVINGOBJECT, this))
        {
            AddProcess(this, RM_CLEAROBJECTS, 0, 0, 0, 0, NULL);
            AddProcess(this, RM_CHANGEMAP, 0, 0, 0, 0, NULL);

            AddRefMsg(RM_SPACEMOVE_SHOW, m_Direction, m_CurrX, m_CurrY, 0, NULL);
        }
    }
}


// update the view range for each object
// 1. mark all obects in the list as 0, means in-visible
// 2. update the range
//      1. detected new object: create a record and mark it as 2
//      2. still in the list, mark it as 1
// 3. for new detected message, send message and mark as 1
// 4. for objects marked as 0, send message and delete it
void CharObject::SearchViewRange()
{
    int nStartX = m_CurrX - m_ViewRange;
    int nStopX  = m_CurrX + m_ViewRange;
    int nStartY = m_CurrY - m_ViewRange;
    int nStopY  = m_CurrY + m_ViewRange;

    // we won't exam the validation of object here
    for(auto &stRecord: m_VisibleObjectList){
        std::get<0>(stRecord) = 0;
    }

    for(auto &stRecord: m_VisibleItemList){
        std::get<0>(stRecord) = 0;
    }

    for(auto &stRecord: m_VisibleEventList){
        std::get<0>(stRecord) = 0;
    }

    // do the update
    // need to source global object/item/event list
    extern MonoServer *g_MonoServer;
    for(int nX = nStartX; nX <= nStopX; nX++){
        for(int nY = nStartY; nY <= nStopY; nY++){
            std::forward_list<CharObjectID> stList;
            m_Map->GetObjectList(nX, nY, &stList);
            auto fnQueryObject = [&m_VisibleObjectList, nX, nY](
                    uint8_t nType, uint32_t nID, uint32_t nAddTime){
                switch(nType){
                    case OT_MOVINGOBJECT:
                        {
                            // only check exact at this time it's still there
                            // only grab it, check every time
                            if(g_MonoServer->CharObjectCheck(nID, nAddTime)){
                                bool bFind = false;
                                for(auto &stRecord: m_VisibleObjectList){
                                    if(true
                                            && std::get<1>(stRecord.ID)      == nID
                                            && std::get<1>(stRecord.Type)    == nType
                                            && std::get<1>(stRecord.AddTime) == nAddTime){
                                        std::get<0>(stRecord) = 1;
                                        bFind = true;
                                    }
                                }
                                if(!bFind){
                                    m_VisibleObjectList.emplace_front(2, {nType, nID, nAddTime});
                                }
                            }else{
                                m_Map->DeleteCharObject(nX, nY, nID, nAddTime);
                            }
                            break;
                        }
                    case OT_ITEMOBJECT:
                        {
                            if(g_MonoServer->GetTickCount() - nAddTime > 60 * 60 * 1000){
                                g_MonoServer->DeleteItem(m_Map->ID(), nID, nAddTime);
                            }else{
                                bool bFind = false;
                                for(auto &stRecord: m_VisibleObjectList){
                                    if(true
                                            && std::get<1>(stRecord.ID)      == nID
                                            && std::get<1>(stRecord.Type)    == nType
                                            && std::get<1>(stRecord.AddTime) == nAddTime){
                                        std::get<0>(stRecord) = 1;
                                        bFind = true;
                                    }
                                }
                                if(!bFind){
                                    m_VisibleObjectList.emplace_front(2, {nType, nID, nAddTime});
                                }
                            }
                            break;
                        }
                    case OT_EVENTOBJECT:
                        {
                            pEvent = (CEvent*)pOSObject->pObject;

                            if(pEvent->m_Visible){
                                UpdateVisibleEvent(pEvent);
                            }
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
            };

            m_Map->QueryObject(nX, nY, fnQueryObject);
        }
    }

    auto fnTestObject = [this](const auto &stRecord){
        switch(std::get<0>(stRecord)){
            case 0:
                {
                    if(Type(OT_HUMAN)){
                        extern MonoServer *g_MonoServer;
                        if(auto pSession = g_MonoServer->SessionGrab()){
                            SMCharObjectDisappear stDisappear = {
                                .ID      = stRecord.ID,
                                .AddTime = stRecord.ID,
                            };
                            pSession->Send(RM_DISAPPEAR, stDisappear);
                        }
                    }
                    return true;
                }
            case 1:
                {
                    return false;
                }
            case 2:
                {
                    if(Type(OT_HUMAN)){
                        extern MonoServer *g_MonoServer;
                        if(auto pObject = g_MonoServer->CharObjectGrab(nID, nAddTime)){
                            if(pObject->Dead()){
                                g_MonoServer->Send(RM_DEATH, ...);
                            }else{
                                g_MonoServer->Send(RM_TURN);
                            }
                        }
                    }
                    return true;
                }
        }
    };

    m_VisibleObjectList.erase(
            std::remove_if(m_VisibleObjectList.begin(), m_VisibleObjectList.end(), fnTestObject));

    auto fnTestItem = [](){
        switch(std::get<0>(stRecord)){
            case 0:
                {
                    // 1. send message of RM_ITEMHIDE
                    // 2. remove item from current view list
                    // 3. don't try to delete it here, doesn't see it doesn't means it doesn't exist
                    return true;
                }
            case 1:
                {
                    return false;
                }
            case 2:
                {
                    // 1. send message of RM_ITEMSHOW
                    // 2. remove item from current view list
                    // 3. don't try to delete it here, doesn't see it doesn't means it doesn't exist
                    return false;
                }
            default:
                {
                    return false;
                }
        }
    };
    m_VisibleItemList.erase();

    auto fnTestItem = [](){
        switch(std::get<0>(stRecord)){
            case 0:
                {
                    // 1. send message of RM_ITEMHIDE
                    // 2. remove item from current view list
                    // 3. don't try to delete it here, doesn't see it doesn't means it doesn't exist
                    return true;
                }
            case 1:
                {
                    return false;
                }
            case 2:
                {
                    // 1. send message of RM_ITEMSHOW
                    // 2. remove item from current view list
                    // 3. don't try to delete it here, doesn't see it doesn't means it doesn't exist
                    return false;
                }
            default:
                {
                    return false;
                }
        }
    };
    m_VisibleEventList.erase();
}

bool CharObject::Ghost()
{
    return m_Ghost;
}

void CharObject::Ghost(bool bGhost)
{
    if(!bGhost){ return; }

    m_Ghost = true;
    extern EventTaskHub *g_EventTaskHub;
    g_EventTaskHub->Add(3 * 60 * 1000, [m_UID, m_AddTime](){
            extern MonoServer *g_MonoServer;
            g_MonoServer->DeleteCharObject(m_UID, m_AddTime);
            });

    m_Map->DeleteCharObject(m_CurrX, m_CurrY, OS_MOVINGOBJECT, m_UID, m_AddTime);
    extern MonoServer *g_MonoServer;
    g_MonoServer->Send(RM_DISAPPEAR);
}

void CharObject::TurnXY(int nX, int nY)
{
    AddRefMsg(RM_TURN, nDir, m_CurrX, m_CurrY, 0, m_Name);

    m_Direction = nDir;
}

bool CharObject::WalkXY(int nX, int nY)
{
    if(!m_Map->CanMove(nX, nY)){
        return false;
    }

    if(!m_Map->ObjectMove(m_CurrX, m_CurrY, nX, nY, this)){
        return false;
    }

    // 2. boardcast this event
    const SMCharObjectWalk stWalk = {
        .UID     = m_ObjectID.UID,
        .AddTime = m_ObjectID.AddTime,
        .X       = nX,
        .Y       = nY,
    };

    auto fnTask = [this, stWalk](CharObjectID stThis, CharObjectID stNeighbor){
        if(auto pObject = g_MonoServer->CharObjectGrab(stNeighbor)){
            if(pObject->Type(OT_PLAYER)){
                if(auto pSession = g_MonoServer->SessionGrab(m_ObjectID.GUID)){
                    pSession->Send(SM_CHAROBJECTWALK, stWalk);
                    pSession->Unlock();
                }
            }
        }
    };

    RangeTask(RT_AROUND, fnTask);

    return true;
}

bool CharObject::RunXY(int nX, int nY)
{
    // 1. report to map to re-assign object to proper cell etc.
    if(!m_Map->ObjectMove(m_CurrX, m_CurrY, nX, nY, this)){
        return false;
    }

    // 2. boardcast this event
    const SMCharObjectRun stRun = {
        .UID     = m_ObjectID.UID,
        .AddTime = m_ObjectID.AddTime,
        .X       = nX,
        .Y       = nY,
    };

    auto fnTask = [this, stRun](CharObjectID stThis, CharObjectID stNeighbor){
        if(auto pObject = g_MonoServer->CharObjectGrab(stNeighbor)){
            if(pObject->Type(OT_PLAYER)){
                if(auto pSession = g_MonoServer->SessionGrab(m_ObjectID.GUID)){
                    pSession->Send(SM_CHAROBJECTRUN, stRun);
                    pSession->Unlock();
                }
            }
        }
    };

    RangeTask(RT_AROUND, fnTask);
    return true;
}

bool CharObject::RangeTask(int nRangeType,
        std::function<void(CharObjectID, CharObjectID)> fnOp)
{
    switch(nRangeType){
        case RT_AROUND:
            {
                if(m_CacheObjectList.empty()
                        || g_MonoServer->GetTickCount() - m_CacheTick > SYS_CACHETICK){
                    int nStartX = m_CurrX - SYS_RANGEX;
                    int nStopX  = m_CurrX + SYS_RANGEX;
                    int nStartY = m_CurrY - SYS_RANGEY;
                    int nStopY  = m_CurrY + SYS_RANGEY;

                    m_CacheObjectList.clear();
                    for(int nX = nStartX; nX <= nStopX; ++nX){
                        for(int nY = nStartY; nY <= nStopY; ++nY){
                            std::forward_list<CharObjectID> stList;
                            m_Map->GetObjectList(nX, nY, &stList);
                            m_CacheObjectList.insert(
                                    m_CacheObjectList.end(), stList.begin(), stList.end());
                        }
                    }
                }

                for(auto &stID: m_CacheObjectList){
                    fnOp(m_ObjectID, stID);
                }

                return true;
            }

        case RT_MAP:
            {
                return true;
            }
        case RT_SERVER:
            {
                return true;
            }
        default:
            {
                return false;
            }
    }

    return false;
}

ObjectLockGuard CharObject::CheckOut(bool bLockIt, uint32_t nID, uint32_t nAddTime)
{
    if(nID == 0 || nAddTime == 0){
        return {nullptr, false};
    }

    std::lock_guard<std::mutex> stLockGuard<m_CharObjectHubLock>;
    auto p = m_CharObjectHub.find(nID);

    if(p == m_CharObjectHub.end()){
        return {nullptr, false};
    }else{
        if(nAddTime != p.second.second){
            return {nullptr, false};
        }
    }

    // OK now we get it
    if(bLockIt){
        p.second.first->Lock();
    }

    return {p.second.first, bLockIt};
}

void CharObject::Lock()
{
    extern MonoServer *g_MonoServer;
    return g_MonoServer->LockObject(m_ID, m_AddTime);

}

bool CharObject::Target(uint32_t nID, uint32_t nAddTime)
{
    extern MonoServer *g_MonoServer;
    if(g_MonoServer->CheckOut<CharObject>(nID, nAddTime)){
        m_Target = std::tuple<uint32_t, uint32_t>(nID, nAddTime);
        return true;
    }
    return false;
}
