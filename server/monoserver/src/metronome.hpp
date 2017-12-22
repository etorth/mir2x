/*
 * =====================================================================================
 *
 *       Filename: metronome.hpp
 *        Created: 04/21/2016 17:29:38
 *  Last Modified: 05/10/2017 00:38:28
 *
 *    Description: generate time tick as MessagePack for actor
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

#include <mutex>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <Theron/Theron.h>

#include "messagepack.hpp"
#include "eventtaskhub.hpp"

class Metronome final: public Theron::Receiver
{
    private:
        uint32_t               m_EventTaskID;           // operation ID in the scheduler
        std::function<void()> *m_EventTaskFunc;         // 

    private:
        std::mutex                   m_AddressVLock;    // single lock to protect all
        std::vector<Theron::Address> m_AddressV;        // only actor address, no receiver's

    public:
        Metronome(uint32_t nTick)
            : Theron::Receiver()
            , m_EventTaskID(0)
            , m_EventTaskFunc(nullptr)
            , m_AddressVLock()
            , m_AddressV()
        {
            // the metronome is immediately ready after creation
            extern EventTaskHub *g_EventTaskHub;
            m_EventTaskFunc = new std::function<void()>([this, nTick]()
            {
                {
                    // 1. lock the whole class so no address can be added in
                    std::lock_guard<std::mutex> stLockGuard(m_AddressVLock);

                    // 2. send time ticks to all address taking in charge
                    extern Theron::Framework *g_Framework;
                    size_t nIndex = 0;

                    while(nIndex < m_AddressV.size()){
                        if(true
				&& m_AddressV[nIndex] != Theron::Address::Null()
                                // must use MessagePack(MPK_METRONOME)
                                // otherwise Theron::Framework::Send<T>(MPK_METRONOME) takes T as int
                                && g_Framework->Send(MessagePack(MPK_METRONOME), GetAddress(), m_AddressV[nIndex])){
                            // current address is valid
                            // send message done and jump to next
                            nIndex++;
                            continue;
                        }

                        // invalid address
                        // could be null or deleted already
                        std::swap(m_AddressV[nIndex], m_AddressV.back());
                        m_AddressV.pop_back();
                    }
                }

                // 3. record the ID of next invocation
                //    every invocation will remove current task handler
                m_EventTaskID = g_EventTaskHub->Add(nTick, *m_EventTaskFunc);
            });

            // don't need to lock now since it's in constructor
            m_EventTaskID = g_EventTaskHub->Add(nTick, *m_EventTaskFunc);
        }

        virtual ~Metronome()
        {
            std::lock_guard<std::mutex> stLockGuard(m_AddressVLock);

            // 1. now the handler won't invoke again
            //    here we need to lock the class
            //    since m_EventTaskID may change during invocaitoin of m_EventTaskFunc
            extern EventTaskHub *g_EventTaskHub;
            g_EventTaskHub->Dismiss(m_EventTaskID);

            // 2. delete the handler
            delete m_EventTaskFunc;
        }

    public:
        void Activate(const Theron::Address &rstNewAddress)
        {
            std::lock_guard<std::mutex> stLockGuard(m_AddressVLock);
            if(std::find(m_AddressV.begin(), m_AddressV.end(), rstNewAddress) == m_AddressV.end()){
                m_AddressV.push_back(rstNewAddress);
            }
        }
};
