/*
 * =====================================================================================
 *
 *       Filename: fpsmonitor.hpp
 *        Created: 12/11/2018 04:01:50
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
#include <deque>
#include <cstdint>
#include <cstddef>
#include <SDL2/SDL.h>
#include "fflerror.hpp"

class FPSMonitor
{
    private:
        const size_t m_size;
        std::deque<uint32_t> m_timeStamp;

    public:
        FPSMonitor(size_t monitorSize = 0)
            : m_size(monitorSize ? monitorSize : 128)
        {}

    public:
        void update()
        {
            m_timeStamp.push_back(SDL_GetTicks());
            if(m_timeStamp.size() > m_size){
                m_timeStamp.pop_front();
            }
        }

    public:
        size_t fps() const
        {
            if(m_timeStamp.size() < 2){
                return 0;
            }
            return 1000 * m_timeStamp.size() / (m_timeStamp.back() + 1 - m_timeStamp.front());
        }
};
