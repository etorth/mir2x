/*
 * =====================================================================================
 *
 *       Filename: basehub.hpp
 *        Created: 04/03/2016 03:49:00
 *  Last Modified: 05/24/2016 15:10:20
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

#include <thread>
#include <atomic>

template<typename Derived>
class BaseHub
{
    protected:
        // defind thread state of the hub
        //     F:   terminated
        //     T:   running
        //
        // won't make an enum for it since there is only three possibility
        std::thread       m_Thread;
        std::atomic<bool> m_ThreadState;

    public:
        BaseHub()
            : m_ThreadState(false)
        {}

        virtual ~BaseHub() = default;

    public:
        void Launch()
        {
            // 1. if running then do nothing
            if(State()){ return; }

            // ok it's stopped

            // 2. make sure there is no MainLoop() running
            //    for one condition: we shutdown the thread and the MainLoop() is unfinished yet
            Join();

            // 3. make it in running state
            State(true);

            // 4. start the thread
            m_Thread = std::thread([this](){ (static_cast<Derived*>(this))->MainLoop(); });
        }

        // this function doesn't make sense since we can't notify the thread
        // however the condition variable is not in this base class, so we
        // actually can't notify it, so... just comment this function out
        // void Suspend()
        // {
        //     State(false);
        // }

        void Join()
        {
            if(m_Thread.joinable()){ m_Thread.join(); }
        }

    public:
        int State()
        {
            return m_ThreadState.load(std::memory_order_relaxed);
        }

        void State(bool nNewState)
        {
            m_ThreadState.store(nNewState, std::memory_order_relaxed);
        }
};
