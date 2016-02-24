/*
 * =====================================================================================
 *
 *       Filename: mapthread.hpp
 *        Created: 02/24/2016 00:08:05
 *  Last Modified: 02/24/2016 00:16:06
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
        MapThread(uint16_t, void *);
        ~MapThread();

    public:
        void Run();
};
