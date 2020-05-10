/*
 * =====================================================================================
 *
 *       Filename: delaycmd.hpp
 *        Created: 05/04/2016 14:13:04
 *    Description: delay cmd for active object
 *                 and field m_count for issue that std::priority_queue not stable
 *
 *                 m_count should be strict ordered if there exist more than one
 *                 DelayCmd object with the same m_tick field
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
        uint32_t m_tick;
        uint32_t m_count;

    private:
        std::function<void()> m_cmd;

    public:
        DelayCmd(uint32_t nTick, uint32_t nCount, const std::function<void()> &fnCmd)
            : m_tick(nTick)
            , m_count(nCount)
            , m_cmd(fnCmd)
        {}

        DelayCmd(uint32_t nTick, const std::function<void()> &fnCmd)
            : DelayCmd(nTick, 0, fnCmd)
        {}

        void operator () () const
        {
            if(m_cmd){
                m_cmd();
            }
        }

        // reverse the sematics of operator < ()
        // by default std::priority_queue<T> is a max-heap for sort()
        bool operator < (const DelayCmd &rstCmd) const
        {
            return (m_tick == rstCmd.m_tick) ? (m_count > rstCmd.m_count) : (m_tick > rstCmd.m_tick);
        }

        uint32_t Tick() const
        {
            return m_tick;
        }
};
