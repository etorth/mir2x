/*
 * =====================================================================================
 *
 *       Filename: player.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 04/14/2016 00:09:02
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

#include "player.hpp"
#include "charobject.hpp"

Player::Player()
    : CharObject()
{
}

bool Player::Friend(CharObject* pCharObject) const
{
    if(!pCharObject || pCharObject == this){ return true; }

    if(pCharObject->Type(CHAROBJECT_ANIMAL)){
        return pCharObject->Friend(this);
    }

    if(pCharObject->Type(CHAROBJECT_HUMAN)){
        if(Mode(MODE_ATTACKALL)){
            return false;
        }

        if(Mode(MODE_PEACE)){
            return true;
        }

        auto pHuman = (Player*)(pCharObject);
        if(pHuman->Mode(MODE_PEACE)){
            return true;
        }
    }

    return false;
}

// update the view range for each object
// 1. mark all obects in the list as 0, means in-visible
// 2. update the range
//      1. detected new object: create a record and mark it as 2
//      2. still in the list, mark it as 1
// 3. for new detected message, send message and mark as 1
// 4. for objects marked as 0, send message and delete it
void Player::SearchViewRange()
{
    int nRange = Range(RANGE_VIEW);
    int nStartX = m_CurrX - nRange;
    int nStopX  = m_CurrX + nRange;
    int nStartY = m_CurrY - nRange;
    int nStopY  = m_CurrY + nRange;

    int nGridX0 = nStartX / 48;
    int nGridY0 = nStartY / 32;
    int nGridX1 = nStopX  / 48;
    int nGridY1 = nStopY  / 32;

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

    // 3. filter the list
    // use while for detailed debug message, otherwise use algorithm template instead
    auto pRecord = m_VisibleObjectList.begin();
    while(pRecord != m_VisibleObjectList.end()){
        switch(std::get<0>(pRecord)){
            case 0:
                {
                    extern Log *g_Log;
                    g_Log->AddLog("Object (%d:%d) is out of range of monster (%d:%d)",
                            std::get<1>(*pRecord), std::get<2>(*pRecord), UID(), AddTime());
                    pRecord = m_VisibleObjectList.erase(pRecord);
                    break;
                }
            case 1:
                {
                    pRecord++;
                    break;
                }
            case 2:
                {
                    extern Log *g_Log;
                    g_Log->AddLog("New object (%d:%d) found in range of monster (%d:%d)",
                            std::get<1>(*pRecord), std::get<2>(*pRecord), UID(), AddTime());
                    std::get<0>(*pRecord) = 1;
                    pRecord++;
                    break;
                }
            default:
                {
                    extern Log *g_Log;
                    g_Log->AddLog("Invalid view tag %d of object (%d:%d) for monster (%d:%d)",
                            std::get<0>(*pRecord),
                            std::get<1>(*pRecord), std::get<2>(*pRecord), UID(), AddTime());
                    pRecord = m_VisibleObjectList.erase(pRecord);
                    break;
                }
        }
    }
}

void Player::Ghost(bool bGhost)
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

void Player::TurnXY(int nX, int nY)
{
    AddRefMsg(RM_TURN, nDir, m_CurrX, m_CurrY, 0, m_Name);

    m_Direction = nDir;
}

bool Player::WalkXY(int nX, int nY)
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

bool Player::RunXY(int nX, int nY)
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

bool Player::RangeTask(int nRangeType,
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
