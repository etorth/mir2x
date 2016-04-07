/*
 * =====================================================================================
 *
 *       Filename: servermap.cpp
 *        Created: 04/06/2016 08:52:57 PM
 *  Last Modified: 04/07/2016 00:48:47
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

bool ServerMap::CanMove(int nX, int nY, int nR)
{
    if(false
            || !(nR > 0)
            || !m_Mir2xMap.Valid()
            // || !m_Mir2xMap.ValidP(nX, nY)
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

        if(!m_Mir2xMap.CanWalk()){
            continue;
        }

        for(auto pObject: mGridObjectList[nGridX][nGridY]){
            if(pObject->Type == OT_MOVINGOBJECT){
                auto pCharObject = (CCharObject*)pObject->Object;
                if(!pCharObject){ continue; }

                if(true
                        && !pCharObject->Dead()
                        && !pCharObject->Inspector()
                        && !pCharObject->Hide()){
                    if(CircleOverlap(nX, nY, nR,
                                pCharObject->X() , pCharObject->Y(), pCharObject->R())){
                        return false;
                    }
                }
            }
        }
    }

    return true;
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

bool ServerMap::MoveToMovingObject(int nX, int nY, int nTargetX, int nTargetY, CCharObject* pObject)
{
    bool			fRet = FALSE;

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


bool ServerMap::AddNewObject(int nX, int nY, uint8_t btType, void *pNewObject)
{
    int nGridX = nX / 48;
    int nGridY = nY / 32;

    if(m_Mir2xMap.ValidP(nX, nY) && m_Mir2xMap.CanWalk(nGridX, nGridY)){
        extern MonoServer *g_Monoserver;
        m_GridObjectList[nGridY][nGridX].emplace_front(nType, pNewObject, g_Monoserver->GetTick());
        return true;
    }
    return false;
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

ItemObject* ServerMap::GetItemObject(int nX, int nY)
{
    if(false
            || !m_Mir2xMap.ValidP(nX, nY)
            || !m_Mir2xMap.CanWalk(nX / 48, nY / 32)){
        return false;
    }

    int nGirdX = nX / 48;
    int nGridY = nY / 32;

    for(auto pObject: mGridObjectList[nGridX][nGridY]){
        if(pObject->Type == OT_ITEMOBJECT){
            return (EventObject*)(pObject->Object);
        }
    }

    return nullptr;
}

EventObject *ServerMap::GetEventObject(int nX, int nY)
{
    int nGirdX = nX / 48;
    int nGridY = nY / 32;
    if(m_Mir2xMap.ValidP(nX, nY) && m_Mir2xMap.CanWalk(nGridX, nGridY)){
        auto p = std::find_if(
                m_GridObjectList[nGridX][nGridY].begin(),
                m_GridObjectList[nGridX][nGridY].end(),
                [](const MapObject &rstObject) -> bool {
                    return rstObject.Type == OT_EVENTOBJECT;
                });
        if(p != m_GridObjectList[nGridY][nGridX].end()){
            return p->Object;
        }
    }

    return nullptr;
}

bool ServerMap::CanSafeWalk(int nX, int nY)
{
    int nGirdX = nX / 48;
    int nGridY = nY / 32;

    if(m_Mir2xMap.ValidP(nX, nY) && m_Mir2xMap.CanWalk(nGridX, nGridY)){
        if(std::find_if(
                m_GridObjectList[nGridX][nGridY].begin(), m_GridObjectList[nGridX][nGridY].end(),
                [nType, pObject](const MapObject &rstObject) -> bool {
                    if((pObject->Type == OT_EVENTOBJECT)
                            && (((EventObject*)pObject->Object)->Damage() > 0)){
                        return false;
                    }
                }
         ) != m_GridObjectList[nGridY][nGridX].end()){
            return false;
        }
    }
    return true;
}

bool ServerMap::RemoveObject(int nX, int nY, uint8_t nType, void *pObject)
{
    int nGirdX = nX / 48;
    int nGridY = nY / 32;
    if(m_Mir2xMap.ValidP(nX, nY) && m_Mir2xMap.CanWalk(nGridX, nGridY)){
        std::remove_if(
                m_GridObjectList[nGridX][nGridY].begin(), m_GridObjectList[nGridX][nGridY].end(),
                [nType, pObject](const MapObject &rstObject) -> bool {
                    return (rstObject.Type == nType && pObject == rstObject.Object);
                }
        );
    }
}
