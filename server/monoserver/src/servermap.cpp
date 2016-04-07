/*
 * =====================================================================================
 *
 *       Filename: servermap.cpp
 *        Created: 04/06/2016 08:52:57 PM
 *  Last Modified: 04/06/2016 23:16:48
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

    m_ObjectList.clear();
    m_ObjectList.resize(nH);
    for(auto &stLine: m_ObjectList){
        stLine.resize(nW);
    }
}

bool ServerMap::CanMove(int nX, int nY, int nR)
{
    if(false
            || !(nR > 0)
            || !m_Mir2xMap.Valid()
            || !m_Mir2xMap.ValidP(nX, nY)
            || !m_Mir2xMap.ValidC(nX, nY)){
        return false;
    }

    int nCenterX  = nX / 48;
    int nCenterY  = nY / 32;
    int nGridSize = nR / 32 + 1; // to make it safe

    RotateCoord stRotateCoord;

    stRotateCoord.Reset();
    while(stRotateCoord.Forward()){
        int nGridX = stRotateCoord.X();
        int nGridY = stRotateCoord.Y();

    }
    for(stRotateCoord.Reset(); stRotateCoord.Forward())

    int nSize = 
    BOOL			fRet = FALSE;
    CMapCellInfo*	pMapCellInfo = GetMapCellInfo(nX, nY);
    CCharObject*	pCharObject;
    _LPTOSOBJECT	pOSObject;

    if (pMapCellInfo)
    {
        if (pMapCellInfo->m_chFlag & 0x01)
        {
            fRet = TRUE;

            //			EnterCriticalSection(&pMapCellInfo->m_cs);

            if (pMapCellInfo->m_xpObjectList)
            {	
                if (pMapCellInfo->m_xpObjectList->GetCount())
                {
                    PLISTNODE pListNode = pMapCellInfo->m_xpObjectList->GetHead();

                    while (pListNode)
                    {
                        pOSObject = (_LPTOSOBJECT)pMapCellInfo->m_xpObjectList->GetData(pListNode);

                        if (pOSObject->btType == OS_MOVINGOBJECT)
                        {
                            if (pCharObject = (CCharObject*)pOSObject->pObject)
                            {
                                if (!pCharObject->m_fIsDead && !pCharObject->m_fInspector && !pCharObject->m_fHideMode)
                                {
                                    if (!fFlag) 
                                    {
                                        fRet = FALSE;
                                        break;
                                    }
                                }
                            }
                        }

                        pListNode = pMapCellInfo->m_xpObjectList->GetNext(pListNode);
                    } // while (pListNode)
                } // if (pMapCellInfo->m_pObjectList.GetCount())
            }

            //			LeaveCriticalSection(&pMapCellInfo->m_cs);
        }
    }

    return fRet;
}

int ServerMap::CheckDoorEvent(int nX, int nY, int &nEvent)
{
    CMapCellInfo*	pMapCellInfo = GetMapCellInfo(nX, nY);

    if (pMapCellInfo)
    {
        if (pMapCellInfo->m_sLightNEvent & 0x02)		// Door Event
        {
            nEvent = ((pMapCellInfo->m_sLightNEvent & 0x3FFF) >> 4);

            if (pMapCellInfo->m_sLightNEvent & 0x08)	// Event
            {
                if (pMapCellInfo->m_sLightNEvent & 0xC000)
                    return _DOOR_MAPMOVE_BACK;
                else
                    return _DOOR_MAPMOVE_FRONT;
            }

            return _DOOR_OPEN;
        }
    }

    return _DOOR_NOT;
}

int ServerMap::CheckEvent(int nX, int nY)
{
    CMapCellInfo*	pMapCellInfo = GetMapCellInfo(nX, nY);

    if (pMapCellInfo)
    {
        if (pMapCellInfo->m_sLightNEvent & 0x04)	// Event
            return ((pMapCellInfo->m_sLightNEvent & 0xC000) >> 4);
    }

    return 0;
}

BOOL ServerMap::MoveToMovingObject(int nX, int nY, int nTargetX, int nTargetY, CCharObject* pObject)
{
    BOOL			fRet = FALSE;

    //	CMapCellInfo*	pMapTargetCellInfo	= GetMapCellInfo(nTargetX, nTargetY);
    //	CMapCellInfo*	pMapCurrCellInfo	= GetMapCellInfo(nX, nY);

    //	if (pMapCurrCellInfo && pMapTargetCellInfo)
    //	{
    //		if (pMapTargetCellInfo->m_chFlag & 0x01)
    //		{
    //			if (pMapTargetCellInfo->m_xpObjectList->GetCount())
    //				fRet = FALSE;
    //			else
    //			{
    if (RemoveObject(nX, nY, OS_MOVINGOBJECT, pObject))
    {
        if (AddNewObject(nTargetX, nTargetY, OS_MOVINGOBJECT, pObject))
            fRet = TRUE;
        else
            fRet = FALSE;
    }
    else
    {
#ifdef _DEBUG
        _RPT4(_CRT_WARN, "Remove Failed : %d, %d, %d, %d", nX, nY, nTargetX, nTargetY);
#endif
        fRet = FALSE;
    }
    //			}
    //		} // if ((pMapCellInfo->m_chFlag & 0x01)
    //	} // if (pMapCurrCellInfo && pMapTargetCellInfo)

    return fRet;
}

BOOL ServerMap::RemoveObject(int nX, int nY, BYTE btType, VOID* pRemoveObject)
{
    CMapCellInfo*	pMapCellInfo = GetMapCellInfo(nX, nY);
    PLISTNODE		pListNode;
    _LPTOSOBJECT	pOSObject;

    if (pMapCellInfo)
    {
        //		EnterCriticalSection(&pMapCellInfo->m_cs);

        if (!pMapCellInfo->m_xpObjectList) 
            return FALSE;

        if (pMapCellInfo->m_xpObjectList->GetCount())
        {
            pListNode = pMapCellInfo->m_xpObjectList->GetHead();

            while (pListNode)
            {
                pOSObject = (_LPTOSOBJECT)pMapCellInfo->m_xpObjectList->GetData(pListNode);

                if (pOSObject)
                {
                    if ((pOSObject->pObject == pRemoveObject) && (pOSObject->btType == btType))
                    {
                        pListNode = pMapCellInfo->m_xpObjectList->RemoveNode(pListNode);
                        //						LeaveCriticalSection(&pMapCellInfo->m_cs);
                        return TRUE;
                    }
                }

                pListNode = pMapCellInfo->m_xpObjectList->GetNext(pListNode);
            } // while (pListNode)
        } // if (pMapCellInfo->m_pObjectList.GetCount())

        //		LeaveCriticalSection(&pMapCellInfo->m_cs);
    }

    return FALSE;
}

BOOL ServerMap::AddNewObject(int nX, int nY, BYTE btType, VOID* pAddObject)
{
    CMapCellInfo*	pMapCellInfo = GetMapCellInfo(nX, nY);

    if (pMapCellInfo)
    {
        if (pMapCellInfo->m_chFlag & 0x01)
        {
            if (!pMapCellInfo->m_xpObjectList)
                pMapCellInfo->m_xpObjectList = new CWHList<_LPTOSOBJECT>;

            _LPTOSOBJECT	pOSObject = new _TOSOBJECT;

            if (pOSObject)
            {
                //				EnterCriticalSection(&pMapCellInfo->m_cs);

                pOSObject->btType		= btType;
                pOSObject->pObject		= pAddObject;
                pOSObject->dwAddTime	= GetTickCount();

                pMapCellInfo->m_xpObjectList->AddNewNode(pOSObject);

                //				LeaveCriticalSection(&pMapCellInfo->m_cs);

                return TRUE;
            }
        }
    }

    return FALSE;
}

CCharObject* ServerMap::GetObject(int nX, int nY)
{
    CMapCellInfo*	pMapCellInfo = GetMapCellInfo(nX, nY);
    _LPTOSOBJECT	pOSObject;
    PLISTNODE		pListNode;

    if (pMapCellInfo)
    {
        if (!(pMapCellInfo->m_chFlag & 0x01))
            return NULL;

        //		EnterCriticalSection(&pMapCellInfo->m_cs);

        if (pMapCellInfo->m_xpObjectList)
        {
            if (pMapCellInfo->m_xpObjectList->GetCount())
            {
                pListNode = pMapCellInfo->m_xpObjectList->GetHead();

                while (pListNode)
                {
                    pOSObject = pMapCellInfo->m_xpObjectList->GetData(pListNode);

                    if (pOSObject)
                    {
                        if (pOSObject->btType == OS_MOVINGOBJECT)
                        {
                            CCharObject* pCharObject = (CCharObject*)pOSObject->pObject;

                            if (!pCharObject->m_fIsDead && !pCharObject->m_fIsGhost)
                            {
                                //								LeaveCriticalSection(&pMapCellInfo->m_cs);

                                return pCharObject;
                            }
                        }
                    }

                    pListNode = pMapCellInfo->m_xpObjectList->GetNext(pListNode);
                } // while (pListNode)
            } // if (pMapCellInfo->m_pObjectList.GetCount())
        }

        //		LeaveCriticalSection(&pMapCellInfo->m_cs);
    }

    return NULL;
}

void ServerMap::GetMapObject(int nX, int nY, int nArea, CWHList<CCharObject*>* pList)
{
    int nStartX = nX - nArea;
    int nEndX	= nX + nArea;
    int nStartY = nY - nArea;
    int nEndY	= nY + nArea;

    _LPTOSOBJECT	pOSObject;

    __try
    {
        for (int x = nStartX; x <= nEndX; x++)
        {
            for (int y = nStartY; y <= nEndY; y++)
            {
                if (CMapCellInfo* pMapCellInfo = GetMapCellInfo(x, y))
                {
                    //					EnterCriticalSection(&pMapCellInfo->m_cs);

                    if (pMapCellInfo->m_xpObjectList)
                    {
                        if (pMapCellInfo->m_xpObjectList->GetCount())
                        {
                            PLISTNODE pListNode = pMapCellInfo->m_xpObjectList->GetHead();

                            while (pListNode)
                            {
                                pOSObject = pMapCellInfo->m_xpObjectList->GetData(pListNode);

                                if (pOSObject->btType == OS_MOVINGOBJECT)
                                {
                                    CCharObject* pCharObject = (CCharObject*)pOSObject->pObject;

                                    if (!pCharObject->m_fIsGhost && !pCharObject->m_fIsDead )
                                        pList->AddNewNode(pCharObject);
                                }

                                pListNode = pMapCellInfo->m_xpObjectList->GetNext(pListNode);
                            } // while (pListNode)
                        } // if (pMapCellInfo->m_pObjectList.GetCount())
                    }

                    //					LeaveCriticalSection(&pMapCellInfo->m_cs);
                } // if (pMapCellInfo)
            }// for (y)
        } // for (x)
    }
    __finally
    {
    }
}

void ServerMap::GetAllObject(int nX, int nY, CWHList<CCharObject*>* pList)
{
    _LPTOSOBJECT	pOSObject;

    if (CMapCellInfo* pMapCellInfo = GetMapCellInfo(nX, nY))
    {
        //					EnterCriticalSection(&pMapCellInfo->m_cs);

        if (pMapCellInfo->m_xpObjectList)
        {
            if (pMapCellInfo->m_xpObjectList->GetCount())
            {
                PLISTNODE pListNode = pMapCellInfo->m_xpObjectList->GetHead();

                while (pListNode)
                {
                    pOSObject = pMapCellInfo->m_xpObjectList->GetData(pListNode);

                    if (pOSObject->btType == OS_MOVINGOBJECT)
                    {
                        CCharObject* pCharObject = (CCharObject*)pOSObject->pObject;

                        if (!pCharObject->m_fIsGhost && !pCharObject->m_fIsDead )
                            pList->AddNewNode(pCharObject);
                    }

                    pListNode = pMapCellInfo->m_xpObjectList->GetNext(pListNode);
                } // while (pListNode)
            } // if (pMapCellInfo->m_pObjectList.GetCount())
        }

        //					LeaveCriticalSection(&pMapCellInfo->m_cs);
    } // if (pMapCellInfo)
}

int ServerMap::GetDupCount(int nX, int nY)
{
    CMapCellInfo*	pMapCellInfo = GetMapCellInfo(nX, nY);
    _LPTOSOBJECT	pOSObject;
    PLISTNODE		pListNode;
    int				nCount = 0;

    if (pMapCellInfo)
    {
        //		EnterCriticalSection(&pMapCellInfo->m_cs);

        if (pMapCellInfo->m_xpObjectList)
        {
            if (pMapCellInfo->m_xpObjectList->GetCount())
            {
                pListNode = pMapCellInfo->m_xpObjectList->GetHead();

                while (pListNode)
                {
                    pOSObject = pMapCellInfo->m_xpObjectList->GetData(pListNode);

                    if (pOSObject)
                    {
                        if (pOSObject->btType == OS_MOVINGOBJECT)
                        {
                            CCharObject* pCharObject = (CCharObject*)pOSObject->pObject;

                            if (!pCharObject->m_fIsDead && !pCharObject->m_fIsGhost && !pCharObject->m_fHideMode)
                                nCount++;
                        }
                    }

                    pListNode = pMapCellInfo->m_xpObjectList->GetNext(pListNode);
                } // while (pListNode)
            } // if (pMapCellInfo->m_pObjectList.GetCount())
        }

        //		LeaveCriticalSection(&pMapCellInfo->m_cs);
    }

    return nCount;
}

CMapItem* ServerMap::GetItem(int nX, int nY)
{
    CMapCellInfo*	pMapCellInfo = GetMapCellInfo(nX, nY);
    _LPTOSOBJECT	pOSObject;
    PLISTNODE		pListNode;

    if (pMapCellInfo)
    {
        //		EnterCriticalSection(&pMapCellInfo->m_cs);

        if (!(pMapCellInfo->m_chFlag & 0x01))
            return NULL;

        if (pMapCellInfo->m_xpObjectList)
        {
            if (pMapCellInfo->m_xpObjectList->GetCount())
            {
                pListNode = pMapCellInfo->m_xpObjectList->GetHead();

                while (pListNode)
                {
                    pOSObject = pMapCellInfo->m_xpObjectList->GetData(pListNode);

                    if (pOSObject)
                    {
                        if (pOSObject->btType == OS_ITEMOBJECT)
                        {
                            //							LeaveCriticalSection(&pMapCellInfo->m_cs);
                            return (CMapItem*)pOSObject->pObject;
                        }
                    }

                    pListNode = pMapCellInfo->m_xpObjectList->GetNext(pListNode);
                } // while (pListNode)
            } // if (pMapCellInfo->m_pObjectList.GetCount())
        }

        //		LeaveCriticalSection(&pMapCellInfo->m_cs);
    }

    return NULL;
}

BOOL ServerMap::GetDropPosition(int nOrgX, int nOrgY, int nRange, int &nX, int &nY)
{
    int nLoonCnt	= (4 * nRange) * (nRange + 1);

    for (int i = 0; i < nLoonCnt; i++)
    {
        nX = nOrgX + g_SearchTable[i].x;
        nY = nOrgY + g_SearchTable[i].y;

        if (GetItem(nX, nY) == NULL)
            return TRUE;
    }

    nX = nOrgX;
    nY = nOrgY;

    return FALSE;
}

CEvent* ServerMap::GetEvent(int nX, int nY)
{
    CMapCellInfo*	pMapCellInfo = GetMapCellInfo(nX, nY);
    _LPTOSOBJECT	pOSObject;
    PLISTNODE		pListNode;

    if (pMapCellInfo)
    {
        //		EnterCriticalSection(&pMapCellInfo->m_cs);

        if (!(pMapCellInfo->m_chFlag & 0x01))
            return NULL;

        if (pMapCellInfo->m_xpObjectList)
        {
            if (pMapCellInfo->m_xpObjectList->GetCount())
            {
                pListNode = pMapCellInfo->m_xpObjectList->GetHead();

                while (pListNode)
                {
                    pOSObject = pMapCellInfo->m_xpObjectList->GetData(pListNode);

                    if (pOSObject)
                    {
                        if (pOSObject->btType == OS_EVENTOBJECT)
                        {
                            //							LeaveCriticalSection(&pMapCellInfo->m_cs);
                            return (CEvent*)pOSObject->pObject;
                        }
                    }

                    pListNode = pMapCellInfo->m_xpObjectList->GetNext(pListNode);
                } // while (pListNode)
            } // if (pMapCellInfo->m_pObjectList.GetCount())
        }

        //		LeaveCriticalSection(&pMapCellInfo->m_cs);
    }

    return NULL;
}

BOOL ServerMap::CanSafeWalk(int nX, int nY)
{
    CMapCellInfo*	pMapCellInfo = GetMapCellInfo(nX, nY);
    _LPTOSOBJECT	pOSObject;
    PLISTNODE		pListNode;

    if (pMapCellInfo)
    {
        //		EnterCriticalSection(&pMapCellInfo->m_cs);

        if (!(pMapCellInfo->m_chFlag & 0x01))
            return NULL;

        if (pMapCellInfo->m_xpObjectList)
        {
            if (pMapCellInfo->m_xpObjectList->GetCount())
            {
                pListNode = pMapCellInfo->m_xpObjectList->GetHead();

                while (pListNode)
                {
                    pOSObject = pMapCellInfo->m_xpObjectList->GetData(pListNode);

                    if (pOSObject)
                    {
                        if (pOSObject->btType == OS_EVENTOBJECT)
                        {
                            //							LeaveCriticalSection(&pMapCellInfo->m_cs);
                            if (((CEvent*)pOSObject->pObject)->m_nDamage > 0)
                                return FALSE;
                        }
                    }

                    pListNode = pMapCellInfo->m_xpObjectList->GetNext(pListNode);
                } // while (pListNode)
            } // if (pMapCellInfo->m_pObjectList.GetCount())
        }

        //		LeaveCriticalSection(&pMapCellInfo->m_cs);
    }

    return TRUE;
}
