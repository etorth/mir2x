/*
 * =====================================================================================
 *
 *       Filename: mapthread.hpp
 *        Created: 02/24/2016 00:08:05
 *  Last Modified: 02/27/2016 22:21:28
 *
 *    Description: every map is a thread in server
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
#include <thread>

class MapThread final
{
    public:
        MapThread();
        ~MapThread();

    public:
        bool Load(uint16_t, void *);
        void Run();

    private:
        uint16_t    m_MapID;
        void       *m_MonoServer;
};
