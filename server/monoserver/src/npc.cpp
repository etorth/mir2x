/*
 * =====================================================================================
 *
 *       Filename: npc.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 04/08/2016 22:10:33
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

#include "charobject.hpp"

CharObject::~CharObject()
{
    std::for_each(
            m_VisibleObjectList.first(),
            m_VisibleObjectList.end(),
            [](VisibleObject *pObject){ delete pObject; });
    m_VisibleObjectList.clear();
}

void CharObject::SelectTarget(CharObject* pTargetObject)
{
    extern MonoServer *g_MonoServer;
    m_TargetObject    = pTargetObject;
    m_TargetFocusTime = g_MonoServer->GetTickCount();
}

int CharObject::GetBack()
{
    switch (m_Direction){
        case DR_UP       : return DR_DOWN;
        case DR_DOWN     : return DR_UP;
        case DR_LEFT     : return DR_RIGHT;
        case DR_RIGHT    : return DR_LEFT;
        case DR_UPLEFT   : return DR_DOWNRIGHT;
        case DR_UPRIGHT  : return DR_DOWNLEFT;
        case DR_DOWNLEFT : return DR_UPRIGHT;
        case DR_DOWNRIGHT: return DR_UPLEFT;
        default: break;
    }

    return nDirection;
}

// let's make it very complicated later
// 1. human can't be slave of anyone
// 2. animal can be slave of human or animal
// 3. master of animal can be slave of human or animal
// 4. don't make a loop a->b->c->a
// 5. etc.

bool CharObject::Friend(CharObject* pCharObject)
{
    // assume this or master of this is a human
    auto fnFriendToHuman = [&fnFriendToHuman](CharObject *pObject){
        if(pCharObject->Type(OT_HUMAN)){
            if(pCharObject->UserInfo()){
                switch(pCharObject->UserInfo()->AttackMode()){
                    case AM_PEACE: return true;
                    default:       return false;
                }
            }
            return false;
        }

        if(pCharObject->Type(OT_ANIMAL)){
            if(pCharObject->Master()){
                if(pCharObject->Master()->Type(OT_HUMAN)){
                    return fnFriendToHuman(pCharObject->Master());
                }
            }
        }
    };

    // assume this is an animal or master of this is an animal
    auto fnFriendToAnimal = [&fnFriendToHuman, &fnFriendToAnimal](CharObject *pCharObject){
        if(pCharObject->Type(OT_ANIMAL)){
            if(pCharObject->Master()){
                return fnFriendToAnimal(pCharObject->Master());
            }
            return true;
        }

        if(pCharObject->Type(OT_HUMAN)){
            return false;
        }
    };

    if(Type(OT_HUMAN)){
        return fnFriendToHuman(pCharObject);
    }else{
        auto pMasterObject = pCharObject;
        while(!pMasterObject->Type(OT_HUMAN) && pMasterObject->Master()){
            pMasterObject = pMasterObject->Master();
        }

        if(pMasterObject->Type(OT_HUMAN)){
            return fnFriendToHuman(this);
        }else{
            return fnFriendToAnimal(this);
        }
    }
}

bool CharObject::DropItemDown(_LPTUSERITEMRCD lpTItemRcd, int nRange, bool fIsGenItem)
{
    CMapItem* xpMapItem = new CMapItem;

    xpMapItem->nCount = 1;

    if(fIsGenItem)
    {
        _LPTGENERALITEMRCD lpTGenItemRcd = NULL;

        lpTGenItemRcd = (_LPTGENERALITEMRCD)lpTItemRcd;

        xpMapItem->wLooks       = (WORD)g_pStdItemEtc[lpTGenItemRcd->nStdIndex].dwLooks;
        xpMapItem->btAniCount   = (BYTE)0;

        xpMapItem->pItem        = (LONG)lpTGenItemRcd;
        memmove(xpMapItem->szName, g_pStdItemEtc[lpTGenItemRcd->nStdIndex].szName, memlen(g_pStdItemEtc[lpTGenItemRcd->nStdIndex].szName));
    }
    else
    {
        xpMapItem->wLooks       = (WORD)g_pStdItemSpecial[lpTItemRcd->nStdIndex].dwLooks;
        xpMapItem->btAniCount   = (BYTE)g_pStdItemSpecial[lpTItemRcd->nStdIndex].wAniCount;

        xpMapItem->pItem        = (LONG)lpTItemRcd;

        if(strlen(lpTItemRcd->szPrefixName))
        {
            strcpy(xpMapItem->szName, lpTItemRcd->szPrefixName);
            strcat(xpMapItem->szName, " ");
            strcat(xpMapItem->szName, g_pStdItemSpecial[lpTItemRcd->nStdIndex].szName);
        }
        else
            strcpy(xpMapItem->szName, g_pStdItemSpecial[lpTItemRcd->nStdIndex].szName);
    }

    int nX, nY;

    m_Map->GetDropPosition(m_CurrX, m_CurrY, nRange, nX, nY);
    m_Map->AddNewObject(nX, nY, OS_ITEMOBJECT, (VOID *)xpMapItem);

    AddRefMsg(RM_ITEMSHOW, xpMapItem->wLooks, (int)xpMapItem, nX, nY, xpMapItem->szName);

    return true;
}

void CharObject::Run()
{
    if(m_OpenHealth)
    {
        if(g_MonoServer->GetTickCount() - m_OpenHealthStart > m_OpenHealthTime)
            BreakOpenHealth();
    }

    bool fChg = false;
    bool fNeedRecalc = false;

    for (int i = 0; i < MAX_STATUS_ATTRIBUTE; i++)
    {
        if(m_StateAttrV[i] > 0 && m_StateAttrV[i] < 60000)
        {
            if(g_MonoServer->GetTickCount() - m_StatusTime[i] > 1000)
            {
                m_StateAttrV[i]     -= 1;
                m_StatusTime[i] += 1000;

                if(m_StateAttrV[i] == 0)
                {
                    fChg = true;

                    switch (i)
                    {
                        case STATE_DEFENCEUP:
                            fNeedRecalc = true;
                            SysMsg("방어력 상승 해제", 1);
                            break;
                        case STATE_MAGDEFENCEUP:
                            fNeedRecalc = true;
                            SysMsg("마항력 상승 해제", 1);
                            break;
                            /*                        case STATE_TRANSPARENT:
                                                      begin
BoHumHideMode := false;
end; */
                        case STATE_BUBBLEDEFENCEUP:
                            m_AbilMagicBubbleDefence = false;
                            break;
                    }
                }
            }
        }
    }

    if(fChg)
    {
        m_CharStatus = GetCharStatus();
        AddRefMsg(RM_CHARSTATUSCHANGED, m_HitSpeed/*wparam*/, m_CharStatus, 0, 0, NULL);
    }

    if(fNeedRecalc)
    {
        ((CPlayerObject*)this)->RecalcAbilitys();
        AddProcess(this, RM_ABILITY, 0, 0, 0, 0, NULL);
    }
}

void CharObject::Die()
{
    if(m_IsNeverDie) return;

    m_IncHealing    = 0;
    m_IncHealth     = 0;
    m_IncSpell      = 0;

    m_DeathTime = g_MonoServer->GetTickCount();

    if(m_ObjectType & (_OBJECT_ANIMAL | _OBJECT_HUMAN))
        AddRefMsg(RM_DEATH, m_Direction, m_CurrX, m_CurrY, 1, NULL);

    m_Dead      = true;
}

bool CharObject::GetAvailablePosition(CMirMap* pMap, int &nX, int &nY, int nRange)
{
    if(pMap->CanMove(nX, nY))
        return true;

    int nOrgX       = nX;
    int nOrgY       = nY;
    int nLoonCnt    = (4 * nRange) * (nRange + 1);

    for (int i = 0; i < nLoonCnt; i++)
    {
        nX = nOrgX + g_SearchTable[i].x;
        nY = nOrgY + g_SearchTable[i].y;

        if(pMap->CanMove(nX, nY))
            return true;
    }

    nX = nOrgX;
    nY = nOrgY;

    return false;
}

bool CharObject::GetNextPosition(int nSX, int nSY, int nDir, int nDistance, int& nX, int& nY)
{
    nX = nSX;
    nY = nSY;

    switch (nDir)
    {
        case DR_UP:
            if(nY > (nDistance - 1)) nY -= nDistance;
            break;
        case DR_DOWN:
            if(nY < m_Map->m_MapFH.shHeight - nDistance) nY += nDistance;
            break;
        case DR_LEFT:
            if(nX > (nDistance - 1)) nX -= nDistance;
            break;
        case DR_RIGHT:
            if(nX < m_Map->m_MapFH.shWidth - nDistance) nX += nDistance;
            break;
        case DR_UPLEFT:
            {
                if((nX > nDistance - 1) && (nY > nDistance - 1))
                {
                    nX -= nDistance;
                    nY -= nDistance;
                }

                break;
            }
        case DR_UPRIGHT:
            {
                if((nX > nDistance - 1) && (nY < m_Map->m_MapFH.shHeight - nDistance))
                {
                    nX += nDistance;
                    nY -= nDistance;
                }

                break;
            }
        case DR_DOWNLEFT:
            {
                if((nX < m_Map->m_MapFH.shWidth - nDistance) && (nY > nDistance - 1))
                {
                    nX -= nDistance;
                    nY += nDistance;
                }

                break;
            }
        case DR_DOWNRIGHT:
            {
                if((nX < m_Map->m_MapFH.shWidth - nDistance) && (nY < m_Map->m_MapFH.shHeight - nDistance))
                {
                    nX += nDistance;
                    nY += nDistance;
                }

                break;
            }
    }

    if((m_CurrX == nX) && (m_CurrY == nY))
        return false;

    return true;
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

void CharObject::UpdateDelayProcessCheckParam1(
        CharObject* pCharObject,
        WORD wIdent, WORD wParam, 
        DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData, int nDelay)
{
    _LPTPROCESSMSG  lpProcessMsg;

    int nCount = m_DelayProcessQ.GetCount();

    if(nCount)
    {
        for (int i = 0; i < nCount; i++)
        {
            lpProcessMsg = (_LPTPROCESSMSG)m_DelayProcessQ.PopQ();

            if(lpProcessMsg)
            {
                if(lpProcessMsg->wIdent == wIdent && lpProcessMsg->lParam1 == lParam1)
                {
                    if(lpProcessMsg->pszData)
                    {
                        delete [] lpProcessMsg->pszData;
                        lpProcessMsg->pszData = NULL;
                    }

                    delete lpProcessMsg;
                    lpProcessMsg = NULL;
                }
                else
                    m_DelayProcessQ.PushQ((BYTE *)lpProcessMsg);
            }
        }
    }

    AddDelayProcess(pCharObject, wIdent, wParam, lParam1, lParam2, lParam3, pszData, nDelay);
}

void CharObject::UpdateProcess(CharObject* pCharObject, WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData)
{
    _LPTPROCESSMSG  lpProcessMsg;

    //  EnterCriticalSection(&m_cs);

    int nCount = m_ProcessQ.GetCount();

    if(nCount)
    {
        for (int i = 0; i < nCount; i++)
        {
            lpProcessMsg = (_LPTPROCESSMSG)m_ProcessQ.PopQ();

            if(lpProcessMsg)
            {
                if(lpProcessMsg->wIdent == wIdent)
                {
                    if(lpProcessMsg->pszData)
                    {
                        delete [] lpProcessMsg->pszData;
                        lpProcessMsg->pszData = NULL;
                    }

                    delete lpProcessMsg;
                    lpProcessMsg = NULL;
                }
                else
                    m_ProcessQ.PushQ((BYTE *)lpProcessMsg);
            }
        }
    }

    //  LeaveCriticalSection(&m_cs);

    AddProcess(pCharObject, wIdent, wParam, lParam1, lParam2, lParam3, pszData);
}

void CharObject::AddProcess(CharObject* pCharObject, WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData)
{
    //  EnterCriticalSection(&m_cs);

    _LPTPROCESSMSG lpProcessMsg = new _TPROCESSMSG;

    if(!m_IsGhost)
    {
        if(lpProcessMsg)
        {
            lpProcessMsg->wIdent            = wIdent;
            lpProcessMsg->wParam            = wParam;
            lpProcessMsg->lParam1           = lParam1;
            lpProcessMsg->lParam2           = lParam2;
            lpProcessMsg->lParam3           = lParam3;

            lpProcessMsg->dwDeliveryTime    = 0;

            lpProcessMsg->pCharObject       = pCharObject;

            if(pszData)
            {
                int nLen = memlen(pszData);

                lpProcessMsg->pszData = new char[nLen];
                memmove(lpProcessMsg->pszData, pszData, nLen);
            }
            else
                lpProcessMsg->pszData       = NULL;

            m_ProcessQ.PushQ((BYTE *)lpProcessMsg);
        }
    }

    //  LeaveCriticalSection(&m_cs);
}

void CharObject::AddDelayProcess(CharObject* pCharObject, WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData, int nDelay)
{
    //  EnterCriticalSection(&m_cs);

    _LPTPROCESSMSG lpProcessMsg = new _TPROCESSMSG;

    if(lpProcessMsg)
    {
        lpProcessMsg->wIdent            = wIdent;
        lpProcessMsg->wParam            = wParam;
        lpProcessMsg->lParam1           = lParam1;
        lpProcessMsg->lParam2           = lParam2;
        lpProcessMsg->lParam3           = lParam3;

        lpProcessMsg->dwDeliveryTime    = g_MonoServer->GetTickCount() + nDelay;

        lpProcessMsg->pCharObject       = pCharObject;

        if(pszData)
        {
            int nLen = memlen(pszData);

            lpProcessMsg->pszData = new char[nLen];
            memmove(lpProcessMsg->pszData, pszData, nLen);
        }
        else
            lpProcessMsg->pszData       = NULL;

        m_DelayProcessQ.PushQ((BYTE *)lpProcessMsg);
    }

    //  LeaveCriticalSection(&m_cs);
}

void CharObject::AddRefMsg(
        WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData)
{
    CMapCellInfo*   pMapCellInfo    = NULL;
    CharObject* pCharObject     = NULL;

    int nStartX = m_CurrX - _RANGE_X;
    int nStopX  = m_CurrX + _RANGE_X;
    int nStartY = m_CurrY - _RANGE_Y;
    int nStopY  = m_CurrY + _RANGE_Y;

    if(m_Inspector) return;

    if(g_MonoServer->GetTickCount() - m_CacheTick > _CACHE_TICK || m_CacheObjectList.GetCount() == 0)
    {
        m_CacheObjectList.Clear();

        for (int x = nStartX; x <= nStopX; x++)
        {
            for (int y = nStartY; y <= nStopY; y++)
            {
                if(pMapCellInfo = m_Map->GetMapCellInfo(x, y))
                {
                    if(pMapCellInfo->m_ObjectList)
                    {
                        if(pMapCellInfo->m_ObjectList->GetCount())
                        {
                            PLISTNODE pListNode = pMapCellInfo->m_ObjectList->GetHead();

                            while (pListNode)
                            {
                                _LPTOSOBJECT pOSObject = pMapCellInfo->m_ObjectList->GetData(pListNode);

                                if(pOSObject->btType == OS_MOVINGOBJECT)
                                {
                                    pCharObject = (CharObject*)pOSObject->pObject;

                                    if(!pCharObject->m_IsGhost)
                                    {
                                        if(pCharObject->m_ObjectType & _OBJECT_HUMAN)
                                        {   
                                            pCharObject->AddProcess(this, wIdent, wParam, lParam1, lParam2, lParam3, pszData);                  
                                            m_CacheObjectList.AddNewNode(pCharObject);
                                        }
                                    }
                                }

                                pListNode = pMapCellInfo->m_ObjectList->GetNext(pListNode);
                            } // while (pListNode)
                        } // if(pMapCellInfo->m_ObjectList.GetCount())
                    }
                } // if(pMapCellInfo)
            }// for (y)
        } // for (x)

        m_CacheTick = g_MonoServer->GetTickCount();
    }
    else
    {
        PLISTNODE pListNode = m_CacheObjectList.GetHead();

        while (pListNode)
        {
            CharObject* pCharObject = m_CacheObjectList.GetData(pListNode);

            if(!pCharObject->m_IsGhost)
            {
                if((pCharObject->m_Map = m_Map) && (abs(pCharObject->m_CurrX - m_CurrX) <= 11) && 
                        (abs(pCharObject->m_CurrY - m_CurrY) <= 11))
                {
                    if(pCharObject->m_ObjectType & _OBJECT_HUMAN)
                        pCharObject->AddProcess(this, wIdent, wParam, lParam1, lParam2, lParam3, pszData);
                    //                     end else begin
                    //                      if cret.WantRefMsg and ((msg = RM_STRUCK) or (msg = RM_HEAR) or (msg = RM_DEATH)) then
                    //                       cret.SendMsg (self, msg, wparam, lparam1, lparam2, lparam3, str);
                }
            }

            pListNode = m_CacheObjectList.GetNext(pListNode);
        } // while (pListNode)
    }
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

void CharObject::UpdateVisibleObject(CharObject* pCharObject)
{
    CVisibleObject* pVisibleObject;

    PLISTNODE       pListNode = m_VisibleObjectList.GetHead();

    while (pListNode)
    {
        pVisibleObject = m_VisibleObjectList.GetData(pListNode);

        if(pVisibleObject->pObject == pCharObject)
        {
            pVisibleObject->nVisibleFlag = 1;
            return;
        }

        pListNode = m_VisibleObjectList.GetNext(pListNode);
    } // while (pListNode)

    CVisibleObject* pNewVisibleObject = new CVisibleObject;

    pNewVisibleObject->nVisibleFlag = 2;
    pNewVisibleObject->pObject      = pCharObject;

    m_VisibleObjectList.AddNewNode(pNewVisibleObject);
}

void CharObject::UpdateVisibleItem(int nX, int nY, CMapItem* pMapItem)
{
    CVisibleMapItem* pVisibleItem;

    PLISTNODE       pListNode = m_VisibleItemList.GetHead();

    while (pListNode)
    {
        pVisibleItem = m_VisibleItemList.GetData(pListNode);

        if(pVisibleItem->pMapItem == pMapItem)
        {
            pVisibleItem->nVisibleFlag = 1;
            return;
        }

        pListNode = m_VisibleItemList.GetNext(pListNode);
    } // while (pListNode)

    CVisibleMapItem* pVisibleNewItem = new CVisibleMapItem;

    pVisibleNewItem->nVisibleFlag   = 2;
    pVisibleNewItem->wX             = (WORD)nX;
    pVisibleNewItem->wY             = (WORD)nY;
    pVisibleNewItem->pMapItem       = pMapItem;

    m_VisibleItemList.AddNewNode(pVisibleNewItem);
}

void CharObject::UpdateVisibleEvent(CEvent* pEvent)
{
    CVisibleEvent* pVisibleEvent;

    PLISTNODE       pListNode = m_VisibleEventList.GetHead();

    while (pListNode)
    {
        pVisibleEvent = m_VisibleEventList.GetData(pListNode);

        if(pVisibleEvent->pEvent == pEvent)
        {
            pVisibleEvent->nVisibleFlag = 1;
            return;
        }

        pListNode = m_VisibleEventList.GetNext(pListNode);
    } // while (pListNode)

    CVisibleEvent* pVisibleNewEvent = new CVisibleEvent;

    pVisibleNewEvent->nVisibleFlag  = 2;
    pVisibleNewEvent->pEvent        = pEvent;

    m_VisibleEventList.AddNewNode(pVisibleNewEvent);
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
