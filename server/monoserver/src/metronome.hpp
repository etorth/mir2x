/*
 * =====================================================================================
 *
 *       Filename: metronome.hpp
 *        Created: 04/21/2016 17:29:38
 *  Last Modified: 01/15/2018 10:16:46
 *
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
#include <Theron/Theron.h>
#include "syncdriver.hpp"

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
        std::set<Theron::Address> m_AddressList;

    private:
        SyncDriver m_Driver;

    public:
        Metronome(uint32_t nTick)
            : m_Tick(nTick)
            , m_Lock()
            , m_Thread()
            , m_State(false)
            , m_AddressList()
            , m_Driver()
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
        bool Add(const Theron::Address &rstAddress)
        {
            std::lock_guard<std::mutex> stLockGuard(m_Lock);
            return m_AddressList.insert(rstAddress).second;
        }

        void Remove(const Theron::Address &rstAddress)
        {
            std::lock_guard<std::mutex> stLockGuard(m_Lock);
            m_AddressList.erase(rstAddress);
        }
};
