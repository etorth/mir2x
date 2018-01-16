/*
 * =====================================================================================
 *
 *       Filename: metronome.cpp
 *        Created: 01/14/2018 22:03:25
 *  Last Modified: 01/15/2018 00:01:35
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

#include "metronome.hpp"
#include "monoserver.hpp"

void Metronome::Delay(uint32_t nTick)
{
    if(nTick > 0){
        extern MonoServer *g_MonoServer;
        auto nDoneTick = g_MonoServer->GetTimeTick() + nTick;

        while(nDoneTick > g_MonoServer->GetTimeTick()){
            std::this_thread::sleep_for(std::chrono::milliseconds(1 + (nDoneTick - g_MonoServer->GetTimeTick()) / 2));
        }
    }
}

bool Metronome::Launch()
{
    m_State.store(false);
    if(m_Thread.joinable()){
        m_Thread.join();
    }

    m_State.store(true);
    m_Thread = std::thread([this]()
    {
        while(m_State.load()){

            extern MonoServer *g_MonoServer;
            auto nInitTick = g_MonoServer->GetTimeTick();

            {
                std::lock_guard<std::mutex> stLockGuard(m_Lock);
                for(auto pCurr = m_AddressList.begin(); pCurr != m_AddressList.end();){
                    if(m_Driver.Forward({MPK_METRONOME}, *pCurr)){
                        pCurr = m_AddressList.erase(pCurr);
                    }else{
                        pCurr++;
                    }
                }
            }

            auto nCurrTick = g_MonoServer->GetTimeTick();
            if(nCurrTick < nInitTick + m_Tick){
                Delay(nInitTick + m_Tick - nCurrTick);
            }else{
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Metronome [%p] is over running", this);
            }
        }
    });
    return true;
}
