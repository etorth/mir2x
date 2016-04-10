/*
 * =====================================================================================
 *
 *       Filename: servermap.hpp
 *        Created: 09/03/2015 03:49:00 AM
 *  Last Modified: 04/10/2016 02:26:01
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

#include "asyncobject.hpp"

class CharObject;
class ServerMap
{
    public:
        ServerMap();
        ~ServerMap();

    private:
        std::vector<std::vector<std::forward_list<OBJECTRECORD>>> m_GridObjectRecordListV;
        std::vector<std::vector<std::shared_ptr<std::mutex>>>     m_GridObjectRecordListVLock;

    public:
        bool Load(const char *);
        bool ObjectMove(int, int, CharObject*);
        bool AddObject(int, int, uint8_t, uint32_t, uint32_t);
        bool RemoveObject(int, int, uint8_t, uint32_t, uint32_t);

    private:
        bool GetObjectList(int, int,
                std::forward_list<OBJECTRECORD> *, std::function<bool(uint8_t nType)>);

        bool CanSafeWalk(int, int);
        bool CanMove(int, int, int, uint32_t, uint32_t);

    public:
        bool DropLocation(int, int, int, int *, int *);
};
