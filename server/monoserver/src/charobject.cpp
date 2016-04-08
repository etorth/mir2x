/*
 * =====================================================================================
 *
 *       Filename: charobject.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 04/08/2016 02:25:14
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

        xpMapItem->wLooks		= (WORD)g_pStdItemEtc[lpTGenItemRcd->nStdIndex].dwLooks;
        xpMapItem->btAniCount	= (BYTE)0;

        xpMapItem->pItem		= (LONG)lpTGenItemRcd;
        memmove(xpMapItem->szName, g_pStdItemEtc[lpTGenItemRcd->nStdIndex].szName, memlen(g_pStdItemEtc[lpTGenItemRcd->nStdIndex].szName));
    }
    else
    {
        xpMapItem->wLooks		= (WORD)g_pStdItemSpecial[lpTItemRcd->nStdIndex].dwLooks;
        xpMapItem->btAniCount	= (BYTE)g_pStdItemSpecial[lpTItemRcd->nStdIndex].wAniCount;

        xpMapItem->pItem		= (LONG)lpTItemRcd;

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
                m_StateAttrV[i]		-= 1;
                m_StatusTime[i]	+= 1000;

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

    m_IncHealing	= 0;
    m_IncHealth		= 0;
    m_IncSpell		= 0;

    m_DeathTime	= g_MonoServer->GetTickCount();

    if(m_ObjectType & (_OBJECT_ANIMAL | _OBJECT_HUMAN))
        AddRefMsg(RM_DEATH, m_Direction, m_CurrX, m_CurrY, 1, NULL);

    m_Dead		= true;
}

bool CharObject::GetAvailablePosition(CMirMap* pMap, int &nX, int &nY, int nRange)
{
    if(pMap->CanMove(nX, nY))
        return true;

    int nOrgX		= nX;
    int nOrgY		= nY;
    int nLoonCnt	= (4 * nRange) * (nRange + 1);

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

int	CharObject::GetNextDirection(int nStartX, int nStartY, int nTargetX, int nTargetY)
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
    _LPTPROCESSMSG	lpProcessMsg;

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
    _LPTPROCESSMSG	lpProcessMsg;

    //	EnterCriticalSection(&m_cs);

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

    //	LeaveCriticalSection(&m_cs);

    AddProcess(pCharObject, wIdent, wParam, lParam1, lParam2, lParam3, pszData);
}

void CharObject::AddProcess(CharObject* pCharObject, WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData)
{
    //	EnterCriticalSection(&m_cs);

    _LPTPROCESSMSG lpProcessMsg = new _TPROCESSMSG;

    if(!m_IsGhost)
    {
        if(lpProcessMsg)
        {
            lpProcessMsg->wIdent			= wIdent;
            lpProcessMsg->wParam			= wParam;
            lpProcessMsg->lParam1			= lParam1;
            lpProcessMsg->lParam2			= lParam2;
            lpProcessMsg->lParam3			= lParam3;

            lpProcessMsg->dwDeliveryTime	= 0;

            lpProcessMsg->pCharObject		= pCharObject;

            if(pszData)
            {
                int nLen = memlen(pszData);

                lpProcessMsg->pszData = new char[nLen];
                memmove(lpProcessMsg->pszData, pszData, nLen);
            }
            else
                lpProcessMsg->pszData		= NULL;

            m_ProcessQ.PushQ((BYTE *)lpProcessMsg);
        }
    }

    //	LeaveCriticalSection(&m_cs);
}

void CharObject::AddDelayProcess(CharObject* pCharObject, WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData, int nDelay)
{
    //	EnterCriticalSection(&m_cs);

    _LPTPROCESSMSG lpProcessMsg = new _TPROCESSMSG;

    if(lpProcessMsg)
    {
        lpProcessMsg->wIdent			= wIdent;
        lpProcessMsg->wParam			= wParam;
        lpProcessMsg->lParam1			= lParam1;
        lpProcessMsg->lParam2			= lParam2;
        lpProcessMsg->lParam3			= lParam3;

        lpProcessMsg->dwDeliveryTime	= g_MonoServer->GetTickCount() + nDelay;

        lpProcessMsg->pCharObject		= pCharObject;

        if(pszData)
        {
            int nLen = memlen(pszData);

            lpProcessMsg->pszData = new char[nLen];
            memmove(lpProcessMsg->pszData, pszData, nLen);
        }
        else
            lpProcessMsg->pszData		= NULL;

        m_DelayProcessQ.PushQ((BYTE *)lpProcessMsg);
    }

    //	LeaveCriticalSection(&m_cs);
}

void CharObject::AddRefMsg(
        WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData)
{
    CMapCellInfo*	pMapCellInfo	= NULL;
    CharObject*	pCharObject		= NULL;

    int nStartX = m_CurrX - _RANGE_X;
    int nEndX	= m_CurrX + _RANGE_X;
    int nStartY = m_CurrY - _RANGE_Y;
    int nEndY	= m_CurrY + _RANGE_Y;

    if(m_Inspector) return;

    if(g_MonoServer->GetTickCount() - m_CacheTick > _CACHE_TICK || m_CacheObjectList.GetCount() == 0)
    {
        m_CacheObjectList.Clear();

        for (int x = nStartX; x <= nEndX; x++)
        {
            for (int y = nStartY; y <= nEndY; y++)
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

    PLISTNODE		pListNode = m_VisibleObjectList.GetHead();

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

    pNewVisibleObject->nVisibleFlag	= 2;
    pNewVisibleObject->pObject		= pCharObject;

    m_VisibleObjectList.AddNewNode(pNewVisibleObject);
}

void CharObject::UpdateVisibleItem(int nX, int nY, CMapItem* pMapItem)
{
    CVisibleMapItem* pVisibleItem;

    PLISTNODE		pListNode = m_VisibleItemList.GetHead();

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

    pVisibleNewItem->nVisibleFlag	= 2;
    pVisibleNewItem->wX				= (WORD)nX;
    pVisibleNewItem->wY				= (WORD)nY;
    pVisibleNewItem->pMapItem		= pMapItem;

    m_VisibleItemList.AddNewNode(pVisibleNewItem);
}

void CharObject::UpdateVisibleEvent(CEvent* pEvent)
{
    CVisibleEvent* pVisibleEvent;

    PLISTNODE		pListNode = m_VisibleEventList.GetHead();

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

    pVisibleNewEvent->nVisibleFlag	= 2;
    pVisibleNewEvent->pEvent		= pEvent;

    m_VisibleEventList.AddNewNode(pVisibleNewEvent);
}

void CharObject::SearchViewRange()
{
    int nStartX = m_CurrX - m_ViewRange;
    int nEndX	= m_CurrX + m_ViewRange;
    int nStartY = m_CurrY - m_ViewRange;
    int nEndY	= m_CurrY + m_ViewRange;

    CMapCellInfo*		pMapCellInfo;
    _LPTOSOBJECT		pOSObject;	
    CharObject*		pCharObject;
    CEvent*				pEvent;
    PLISTNODE			pListNode;
    CVisibleObject*		pVisibleObject;
    CVisibleMapItem*	pVisibleItem;
    CVisibleEvent*		pVisibleEvent;

    // Clear VisibleObjectList
    if(m_VisibleObjectList.GetCount())
    {
        pListNode = m_VisibleObjectList.GetHead();

        while (pListNode)
        {
            if(pVisibleObject = m_VisibleObjectList.GetData(pListNode))
                pVisibleObject->nVisibleFlag = 0;

            pListNode = m_VisibleObjectList.GetNext(pListNode);
        }
    }

    // Clear VisibleMapItem
    if(m_VisibleItemList.GetCount())
    {
        pListNode = m_VisibleItemList.GetHead();

        while (pListNode)
        {
            if(pVisibleItem = m_VisibleItemList.GetData(pListNode))
                pVisibleItem->nVisibleFlag = 0;

            pListNode = m_VisibleItemList.GetNext(pListNode);
        }
    }

    // Clear VisibleEvent
    if(m_VisibleEventList.GetCount())
    {
        pListNode = m_VisibleEventList.GetHead();

        while (pListNode)
        {
            if(pVisibleEvent = m_VisibleEventList.GetData(pListNode))
                pVisibleEvent->nVisibleFlag = 0;

            pListNode = m_VisibleEventList.GetNext(pListNode);
        }
    }

    // Search VisibleAllObjectList
    for (int x = nStartX; x <= nEndX; x++)
    {
        for (int y = nStartY; y <= nEndY; y++)
        {
            pMapCellInfo = m_Map->GetMapCellInfo(x, y);

            if(pMapCellInfo)
            {
                if(pMapCellInfo->m_ObjectList)
                {
                    if(pMapCellInfo->m_ObjectList->GetCount())
                    {
                        pListNode = pMapCellInfo->m_ObjectList->GetHead();

                        while (pListNode)
                        {
                            pOSObject = pMapCellInfo->m_ObjectList->GetData(pListNode);

                            if(pOSObject)
                            {
                                if(pOSObject->btType == OS_MOVINGOBJECT)
                                {
                                    pCharObject = (CharObject*)pOSObject->pObject;

                                    if(!pCharObject->m_Inspector && !pCharObject->m_IsGhost && !pCharObject->m_HideMode)
                                        UpdateVisibleObject(pCharObject);
                                }
                                else if(pOSObject->btType == OS_ITEMOBJECT)
                                {
                                    if(g_MonoServer->GetTickCount() - pOSObject->dwAddTime > 60 * 60 * 1000)
                                    {
                                        delete pOSObject;
                                        pOSObject = NULL;

                                        pListNode = pMapCellInfo->m_ObjectList->RemoveNode(pListNode);
                                        continue;
                                    }
                                    else
                                        UpdateVisibleItem(x, y, (CMapItem*)pOSObject->pObject);
                                }
                                else if(pOSObject->btType == OS_EVENTOBJECT)
                                {
                                    pEvent = (CEvent*)pOSObject->pObject;

                                    if(pEvent->m_Visible)
                                        UpdateVisibleEvent(pEvent);
                                }
                            }

                            pListNode = pMapCellInfo->m_ObjectList->GetNext(pListNode);
                        } // while (pListNode)
                    } // if(pMapCellInfo->m_ObjectList.GetCount())
                }
            } // if(pMapCellInfo)
        }// for (y)
    } // for (x)

    if(m_VisibleObjectList.GetCount())
    {
        pListNode = m_VisibleObjectList.GetHead();

        while (pListNode)
        {
            if(pVisibleObject = m_VisibleObjectList.GetData(pListNode))
            {
                if(pVisibleObject->nVisibleFlag == 0)
                {
                    if(!pVisibleObject->pObject->m_HideMode)
                        AddProcess(pVisibleObject->pObject, RM_DISAPPEAR, 0, 0, 0, 0, NULL);

                    delete pVisibleObject;
                    pVisibleObject = NULL;

                    pListNode = m_VisibleObjectList.RemoveNode(pListNode);

                    continue;
                }
                else
                {
                    if(m_ObjectType & _OBJECT_HUMAN)
                    {
                        if(pVisibleObject->nVisibleFlag == 2)	// 2:New Object
                        {
                            if(pVisibleObject->pObject != this)
                            {
                                if(pVisibleObject->pObject->m_Dead)
                                {
                                    AddProcess(pVisibleObject->pObject, RM_DEATH, pVisibleObject->pObject->m_Direction, 
                                            pVisibleObject->pObject->m_CurrX, pVisibleObject->pObject->m_CurrY, 0, NULL);
                                }
                                else
                                    AddProcess(pVisibleObject->pObject, RM_TURN, pVisibleObject->pObject->m_Direction, 
                                            pVisibleObject->pObject->m_CurrX, pVisibleObject->pObject->m_CurrY, 
                                            0, pVisibleObject->pObject->m_Name);
                            }
                        }
                    }
                } // if(pVisibleObject->nVisibleFlag == 0)
            } // if(pVisibleObject = m_VisibleObjectList.GetData(pListNode))

            pListNode = m_VisibleObjectList.GetNext(pListNode);
        } // while (pListNode)
    }

    // Update Map Item
    if(m_VisibleItemList.GetCount())
    {
        pListNode = m_VisibleItemList.GetHead();

        while (pListNode)
        {
            if(pVisibleItem = m_VisibleItemList.GetData(pListNode))
            {
                if(pVisibleItem->nVisibleFlag == 0)
                {
                    AddProcess(this, RM_ITEMHIDE, 0, (LPARAM)pVisibleItem->pMapItem, pVisibleItem->wX, pVisibleItem->wY, NULL);

                    delete pVisibleItem;
                    pVisibleItem = NULL;

                    pListNode = m_VisibleItemList.RemoveNode(pListNode);

                    continue;
                }
                else
                {
                    if(pVisibleItem->nVisibleFlag == 2)	// 2:New Item
                        AddProcess(this, RM_ITEMSHOW, pVisibleItem->pMapItem->wLooks, (LPARAM)pVisibleItem->pMapItem, 
                                pVisibleItem->wX, pVisibleItem->wY, pVisibleItem->pMapItem->szName);	
                } // if(pVisibleObject->nVisibleFlag == 0)
            } // if(pVisibleObject = m_VisibleItemList.GetData(pListNode))

            pListNode = m_VisibleItemList.GetNext(pListNode);
        } // while (pListNode)
    }

    // Update Event Item
    if(m_VisibleEventList.GetCount())
    {
        pListNode = m_VisibleEventList.GetHead();

        while (pListNode)
        {
            if(pVisibleEvent = m_VisibleEventList.GetData(pListNode))
            {
                if(pVisibleEvent->nVisibleFlag == 0)
                {
                    AddProcess(this, RM_HIDEEVENT, 0, (LPARAM)pVisibleEvent->pEvent, pVisibleEvent->pEvent->m_X, pVisibleEvent->pEvent->m_Y);

                    pListNode = m_VisibleEventList.RemoveNode(pListNode);

                    delete pVisibleEvent;
                    pVisibleEvent = NULL;

                    continue;
                }
                else if(pVisibleEvent->nVisibleFlag == 2) 
                {
                    AddProcess(this, RM_SHOWEVENT, pVisibleEvent->pEvent->m_EventType, (LPARAM)pVisibleEvent->pEvent, 
                            MAKELONG(pVisibleEvent->pEvent->m_X, pVisibleEvent->pEvent->m_EventParam), pVisibleEvent->pEvent->m_Y);	
                }
            }

            pListNode = m_VisibleEventList.GetNext(pListNode);
        } // while (pListNode)
    }
}

void CharObject::Disappear()
{
    if(m_Map)
    {
        m_Map->RemoveObject(m_CurrX, m_CurrY, OS_MOVINGOBJECT, this);
        AddRefMsg(RM_DISAPPEAR, 0, 0, 0, 0, NULL);
    }
}

void CharObject::MakeGhost()
{
    m_IsGhost		= true;
    m_GhostTime	= g_MonoServer->GetTickCount();

    Disappear();
}

void CharObject::TurnTo(int nDir)
{
    AddRefMsg(RM_TURN, nDir, m_CurrX, m_CurrY, 0, m_Name);

    m_Direction = nDir;
}

bool CharObject::TurnXY(int nX, int nY, int nDir)
{
    if(m_CurrX == nX && m_CurrY == nY)
    {
        AddRefMsg(RM_TURN, nDir, m_CurrX, m_CurrY, 0, m_Name);

        m_Direction = nDir;

        return true;
    }

    return false;
}

bool CharObject::Walk()
{
    int nX, nY;

}

bool CharObject::WalkTo(int nDir)
{
    int nX, nY;

    WalkNextPos(nDir, nX, nY);

    bool fResult = WalkXY(nX, nY, nDir);

    if(fResult)
    {
        if(m_FixedHideMode)
        {
            if(m_HumHideMode)
                m_StateAttrV[STATE_TRANSPARENT] = 1;
        }
    }

    return fResult;
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
                    int nEndX	= m_CurrX + SYS_RANGEX;
                    int nStartY = m_CurrY - SYS_RANGEY;
                    int nEndY	= m_CurrY + SYS_RANGEY;

                    m_CacheObjectList.clear();
                    for(int nX = nStartX; nX <= nEndX; ++nX){
                        for(int nY = nStartY; nY <= nEndY; ++nY){
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
