/*
 * =====================================================================================
 *
 *       Filename: delaycmd.hpp
 *        Created: 05/04/2016 14:13:04
 *  Last Modified: 06/05/2016 21:41:40
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
#pragma once
#include <cstdint>
#include <functional>

class DelayCmd
{
    private:
        uint32_t m_Tick;
        std::function<void()> m_Cmd;

    public:
        DelayCmd(uint32_t nTick, const std::function<void()> &fnCmd)
            : m_Tick(nTick)
            , m_Cmd(fnCmd)
        {}

        void operator () () const
        {
            if(m_Cmd){ m_Cmd(); }
        }

        // a < b means b is more ``urgent"
        bool operator < (const DelayCmd &rstCmd) const
        {
            return m_Tick > rstCmd.m_Tick;
        }

        uint32_t Tick()
        {
            return m_Tick;
        }

        uint32_t Tick() const
        {
            return m_Tick;
        }
};
