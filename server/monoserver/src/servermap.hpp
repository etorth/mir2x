/*
 * =====================================================================================
 *
 *       Filename: servermap.hpp
 *        Created: 09/03/2015 03:49:00 AM
 *  Last Modified: 04/06/2016 22:29:54
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

#pragma once
#include <cstdint>
#include <forward_list>

enum ObjectType: uint8_t {
    OT_MOVINGOBJECT,
    OT_ITEMOBJECT,
    OT_EVENTOBJECT,
};

typedef struct {
    uint8_t   Type;
    void     *Object;
    uint32_t  AddTime;
}MapObject;

class ServerMap
{
    public:
        ServerMap();
       ~ServerMap();

    private:
        std::vector<std::vector<
            std::forward_list<MapObject*>>>    m_ObjectList;

    public:
        BOOL			LoadMapData(char *pszName);
        VOID			FreeMapData();
        BOOL			CanMove(int nX, int nY, int nFlag = FALSE);
        BOOL			CanSafeWalk(int nX, int nY);

        BOOL			MoveToMovingObject(int nX, int nY, int nTargetX, int nTargetY, CCharObject* pObject);
        BOOL			RemoveObject(int nX, int nY, BYTE btType, VOID* pRemoveObject);
        BOOL			AddNewObject(int nX, int nY, BYTE btType, VOID* pAddObject);

        CEvent*			GetEvent(int nX, int nY);
        CMapItem*		GetItem(int nX, int nY);
        CCharObject*	GetObject(int nX, int nY);
        void			GetMapObject(int nX, int nY, int nArea, CWHList<CCharObject*>* pList);
        void			GetAllObject(int nX, int nY, CWHList<CCharObject*>* pList);
        int				GetDupCount(int nX, int nY);

        BOOL			GetDropPosition(int nOrgX, int nOrgY, int nRange, int &nX, int &nY);

        int				CheckDoorEvent(int nX, int nY, int &nEvent);
        int				CheckEvent(int nX, int nY);

        BOOL			IsValidObject(int nX, int nY, int nCheckRange, CCharObject* pCharObject);

        __inline CMapCellInfo* GetMapCellInfo(int nX, int nY) 
        { 
            if ((nX >= 0 && nX < m_stMapFH.shWidth) && (nY >= 0 && nY < m_stMapFH.shHeight)) 
                return &m_pMapCellInfo[nX * m_stMapFH.shHeight + nY]; 
            return NULL; 
        }
};
