/*
 * =====================================================================================
 *
 *       Filename: latch.hpp
 *        Created: 10/04/2017 11:51:45
 *  Last Modified: 10/04/2017 11:59:20
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
#include <atomic>

class Latch
{
    private:
        const bool m_UseYield;

    private:
        std::atomic_flag m_LatchFlag;

    public:
        Latch(bool bUseYield = true)
            : m_UseYield(bUseYield)
            , m_LatchFlag(ATOMIC_FLAG_INIT)
        {}

    public:
        void lock()
        {
            while(m_LatchFlag.test_and_set(std::memory_order_acquire)){
                if(m_UseYield){
                    std::this_thread::yield();
                }
            }
        }

        void unlock()
        {
            m_LatchFlag.clear(std::memory_order_release);
        }
};
