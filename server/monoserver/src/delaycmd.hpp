/*
 * =====================================================================================
 *
 *       Filename: delaycmd.hpp
 *        Created: 05/04/2016 14:13:04
 *    Description: delay cmd for active object
 *                 and field m_Count for issue that std::priority_queue not stable
 *
 *                 m_Count should be strict ordered if there exist more than one
 *                 DelayCmd object with the same m_Tick field
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
#include <cstdint>
#include <functional>

class DelayCmd
{
    private:
        uint32_t m_Tick;
        uint32_t m_Count;

    private:
        std::function<void()> m_Cmd;

    public:
        DelayCmd(uint32_t nTick, uint32_t nCount, const std::function<void()> &fnCmd)
            : m_Tick(nTick)
            , m_Count(nCount)
            , m_Cmd(fnCmd)
        {}

        DelayCmd(uint32_t nTick, const std::function<void()> &fnCmd)
            : DelayCmd(nTick, 0, fnCmd)
        {}

        void operator () () const
        {
            if(m_Cmd){
                m_Cmd();
            }
        }

        // reverse the sematics of operator < ()
        // by default std::priority_queue<T> is a max-heap for sort()
        bool operator < (const DelayCmd &rstCmd) const
        {
            return (m_Tick == rstCmd.m_Tick) ? (m_Count > rstCmd.m_Count) : (m_Tick > rstCmd.m_Tick);
        }

        uint32_t Tick() const
        {
            return m_Tick;
        }
};
