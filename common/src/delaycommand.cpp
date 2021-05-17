/*
 * =====================================================================================
 *
 *       Filename: delaycommand.cpp
 *        Created: 05/04/2016 14:13:04
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

#include "fflerror.hpp"
#include "raiitimer.hpp"
#include "delaycommand.hpp"

void DelayCommandQueue::addDelay(uint32_t delayTick, std::function<void()> cmd)
{
    if(m_delayCmdQ.empty() && m_addedCmdQ.empty()){
        m_delayCmdIndex = 0;
    }
    else{
        m_delayCmdIndex++;
    }
    m_addedCmdQ.emplace_back(delayTick + hres_tstamp().to_msec(), m_delayCmdIndex, std::move(cmd));
}

void DelayCommandQueue::exec()
{
    const auto fnProcPending = [this]()
    {
        for(auto &addedCmd: m_addedCmdQ){
            m_delayCmdQ.emplace(std::move(addedCmd));
        }
        m_addedCmdQ.clear();
    };

    fnProcPending();
    while(!m_delayCmdQ.empty() && hres_tstamp().to_msec() >= m_delayCmdQ.top().tick()){
        m_delayCmdQ.top()(); // invocation outstanding
        m_delayCmdQ.pop();   //
        fnProcPending();     // above invocation of outstanding delayed command may add new delayed commands
    }
}
