/*
 * =====================================================================================
 *
 *       Filename: basehub.hpp
 *        Created: 04/03/2016 03:49:00
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
#include <thread>

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

    protected:
        BaseHub()
            : m_Thread()
            , m_ThreadState(false)
        {}

        virtual ~BaseHub()
        {
            Join();
        }

    public:
        void Launch()
        {
            // 1. if running then do nothing
            if(State()){
                return;
            }

            // ok it's stopped

            // 2. make sure there is no MainLoop() running
            //    for one condition: we shutdown the thread and the MainLoop() is unfinished yet
            Join();

            // 3. make it in running state
            State(true);

            // 4. start the thread
            //    decide to move invocation of MainLoop() from constructor to Launch()
            //    then don't need use the dirty trick of static_cast<Derived*>(this))->MainLoop()
            m_Thread = std::thread([this](){ MainLoop(); });
        }

    protected:
        void Join()
        {
            if(m_Thread.joinable()){
                m_Thread.join();
            }
        }

    protected:
        int State()
        {
            return m_ThreadState.load(std::memory_order_relaxed);
        }

        void State(bool nNewState)
        {
            m_ThreadState.store(nNewState, std::memory_order_relaxed);
        }

    protected:
        virtual void MainLoop() = 0;
};
