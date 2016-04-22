/*
 * =====================================================================================
 *
 *       Filename: metronome.hpp
 *        Created: 04/21/2016 17:29:38
 *  Last Modified: 04/21/2016 18:30:12
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
#include <Theron/Theron.h>

class Metronome: public Theron::Reveiver
{
    private:
        // TODO only use one lock
        // make it as simple as possible
        bool m_Active;
        std::mutex m_Lock;
        std::vector<Theron::Address> m_AddrV;

    public:
        Metronome(uint32_t nTick)
            : Theron::Reveiver()
            , m_Active(true)
        {
            // immediately ready when created
            extern EventTaskHub *g_EventTaskHub;
            auto pfnRepeatBeat = new std::function<void()>([this, nTick, pfnRepeatBeat](){
                std::lock_guard<std::mutex> stGuard(m_Lock);
                if(m_Active){
                    GenerateBeat();
                    g_EventTaskHub->Add(nTick, *pfnRepeatBeat);
                }else{
                    delete pfnRepeatBeat;
                }
            });

            g_EventTaskHub->Add(nTick, *pfnRepeatBeat);
        }

        virtual ~Metronome()
        {
            std::lock_guard<std::mutex> stGuard(m_Lock);
            m_Active = false;
        }

    public:
        void Activate(Theron::Address stAddr)
        {
            std::lock_guard<std::mutex> stGuard(m_Lock);
            if(std::find(m_AddrV.begin(), m_AddrV.end(), stAddr) == m_AddrV.end()){
                m_AddrV.push_back(stAddr);
            }
        }

    protected:
        void GenerateBeat()
        {
            std::for_each(m_AddrV.begin(), m_AddrV.end(), [this](const Theron::Address &rstAddr){
                // TODO
                // here should we only use the framework bind to the actor?
                // or any framework works? I tested seems any framework will work
                extern Theron::Framework *g_Framework;
                g_Framework->Send(MessagePack(MPK_METRONOME), GetAddress(), rstAddr);
            });
        }
};
