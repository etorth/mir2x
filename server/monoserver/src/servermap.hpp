/*
 * =====================================================================================
 *
 *       Filename: servermap.hpp
 *        Created: 09/03/2015 03:49:00 AM
 *  Last Modified: 04/10/2016 22:58:19
 *
 *    Description: put all non-atomic function as private
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

#include "mir2xmap.hpp"
#include "asyncobject.hpp"

class CharObject;
class ServerMap
{
    private:
        // some shortcuts for internal use only
        // for public API don't use it
        using ObjectRecord     = std::tuple<uint32_t, uint32_t>;
        using ObjectRecordList = std::forward_list<ObjectRecord>;
        using LockPointer      = std::shared_ptr<std::mutex>;
        template<typename T> using Vec2D = std::vector<std::vector<T>>;

    public:
        ServerMap();
        ~ServerMap();

    public:
        uint32_t ID()
        {
            return m_ID;
        }

    private:
        uint32_t m_ID;

    private:
        Mir2xMap m_Mir2xMap;

    public:
        bool ValidC(int nX, int nY)
        {
            return m_Mir2xMap.ValidC(nX, nY);
        }

        bool ValidP(int nX, int nY)
        {
            return m_Mir2xMap.ValidP(nX, nY);
        }


    private:

        Vec2D<ObjectRecordList>  m_GridObjectRecordListV;
        Vec2D<LockPointer>       m_GridObjectRecordListVLock;

    public:
        bool Load(const char *);
        bool ObjectMove(int, int, CharObject*);
        bool AddObject(int, int, uint8_t, uint32_t, uint32_t);
        bool RemoveObject(int, int, uint8_t, uint32_t, uint32_t);

    public:
        bool QueryObject(int, int, const std::function<void(uint8_t, uint32_t, uint32_t)> &);

    private:
        bool GetObjectList(int, int, std::list<ObjectRecord> *, std::function<bool(uint8_t nType)>);

        bool CanSafeWalk(int, int);
        bool CanMove(int, int, int, uint32_t, uint32_t);

    public:
        bool DropLocation(int, int, int, int *, int *);
};
