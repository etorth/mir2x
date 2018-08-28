/*
 * =====================================================================================
 *
 *       Filename: metronome.hpp
 *        Created: 04/21/2016 17:29:38
 *    Description: generate MPK_METRONOME to registered actor addresses
 *                 keep it as simple as possible
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

#include <set>
#include <mutex>
#include <atomic>
#include <cstdint>
#include "dispatcher.hpp"

class Metronome final
{
    private:
        uint32_t m_Tick;

    private:
        std::mutex m_Lock;

    private:
        std::thread m_Thread;

    private:
        std::atomic<bool> m_State;

    private:
        std::set<uint32_t> m_UIDList;

    private:
        Dispatcher m_Dispatcher;

    public:
        Metronome(uint32_t nTick)
            : m_Tick(nTick)
            , m_Lock()
            , m_Thread()
            , m_State(false)
            , m_UIDList()
            , m_Dispatcher()
        {}

    public:
        ~Metronome()
        {
            m_State.store(false);
            if(m_Thread.joinable()){
                m_Thread.join();
            }
        }

    private:
        void Delay(uint32_t);

    public:
        bool Launch();

    public:
        bool Add(uint32_t nUID)
        {
            std::lock_guard<std::mutex> stLockGuard(m_Lock);
            return m_UIDList.insert(nUID).second;
        }

        void Remove(uint32_t nUID)
        {
            std::lock_guard<std::mutex> stLockGuard(m_Lock);
            m_UIDList.erase(nUID);
        }
};
