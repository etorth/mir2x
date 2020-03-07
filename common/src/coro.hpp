/*
 * =====================================================================================
 *
 *       Filename: coro.hpp
 *        Created: 03/07/2020 12:36:32
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
#ifdef _MSC_VER
    #include <windows.h>
#else
    #include "aco.h"
#endif

class Coro
{
    public:
        void coro_yield();
        void coro_resume();
};
