/*
 * =====================================================================================
 *
 *       Filename: delaycommand.hpp
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
#pragma once
#include <cstdint>
#include <functional>
#include "fflerror.hpp"

class DelayCommand
{
    private:
        uint32_t m_tick;
        uint32_t m_index; // to stablize heap sort

    private:
        std::function<void()> m_cmd;

    public:
        DelayCommand(uint32_t tick, uint32_t index, std::function<void()> cmd)
            : m_tick(tick)
            , m_index(index)
            , m_cmd(std::move(cmd))
        {
            if(!m_cmd){
                throw fflerror("DelayCommand is not executable");
            }
        }

        bool operator < (const DelayCommand &other) const
        {
            if(m_tick != other.m_tick){
                return m_tick > other.m_tick;
            }
            return m_index > other.m_index;
        }

        uint32_t tick() const
        {
            return m_tick;
        }

    public:
        void operator () () const
        {
            m_cmd();
        }
};
