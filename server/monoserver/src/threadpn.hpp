/*
 * =====================================================================================
 *
 *       Filename: threadpn.hpp
 *        Created: 04/19/2016 17:36:43
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
#include <system_error>

#include "log.hpp"
#include "threadpool2.hpp"

class ThreadPN: public ThreadPool2
{
    public:
        ThreadPN(size_t nCount)
            : ThreadPool2(nCount)
        {
            extern ThreadPN *g_ThreadPN;
            if(g_ThreadPN){
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_WARNING, "Only one thread pool instance please");
                throw std::error_code();
            }
        }
};
