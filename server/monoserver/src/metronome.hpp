/*
 * =====================================================================================
 *
 *       Filename: metronome.hpp
 *        Created: 04/21/2016 17:29:38
 *  Last Modified: 04/28/2016 23:11:14
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

class Metronome: public Theron::Receiver
{
    private:
        // TODO only use one lock, make it as simple as possible
        //
        uint32_t m_OID;                         // operation ID in the scheduler
        std::mutex m_Lock;                      // single lock to protect all
        std::function<void()> *m_Func;          // 
        std::vector<Theron::Address> m_AddrV;   // only actor address, no receiver's

    public:
        Metronome(uint32_t nTick)
            : Theron::Receiver()
            , m_OID(0)
            , m_Func(nullptr)
        {
            // immediately ready when created
            extern EventTaskHub *g_EventTaskHub;
            m_Func = new std::function<void()>([this, nTick](){
                // 1. lock the whole class so no address can be added in
                std::lock_guard<std::mutex> stGuard(m_Lock);

                // 2. send time ticks to all address taking in charge
                extern Theron::Framework *g_Framework;
                for(const auto &rstAddr: m_AddrV){
                    g_Framework->Send(MessagePack(MPK_METRONOME), GetAddress(), rstAddr);
                }

                // 3. record the ID of next invocation
                m_OID = g_EventTaskHub->Add(nTick, *m_Func);
            });

            // don't need to lock now since it's in ctor
            m_OID = g_EventTaskHub->Add(nTick, *m_Func);
        }

        virtual ~Metronome()
        {
            std::lock_guard<std::mutex> stGuard(m_Lock);

            // 1. now the handler won't invoke again
            //    here we need to lock the class
            //    since m_OID may change during invocaitoin of m_Func
            extern EventTaskHub *g_EventTaskHub;
            g_EventTaskHub->Dismiss(m_OID);

            // 2. delete the handler
            delete m_Func;
        }

    public:
        void Activate(const Theron::Address &rstAddr)
        {
            std::lock_guard<std::mutex> stGuard(m_Lock);
            if(std::find(m_AddrV.begin(), m_AddrV.end(), rstAddr) == m_AddrV.end()){
                m_AddrV.push_back(rstAddr);
            }
        }
};
