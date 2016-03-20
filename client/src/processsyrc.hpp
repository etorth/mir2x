/*
 * =====================================================================================
 *
 *       Filename: processsyrc.hpp
 *        Created: 8/14/2015 2:47:30 PM
 *  Last Modified: 03/20/2016 01:46:16
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

#include <SDL2/SDL.h>
#include "process.hpp"
#include <atomic>

class ProcessSyrc: public Process
{
    private:
        std::atomic<int> m_Ratio;

    public:
        ProcessSyrc();
        virtual ~ProcessSyrc();

    public:
        int ID()
        {
            return PROCESSID_SYRC;
        }

    public:
        void Update(double);
        void Draw();
        void ProcessEvent(const SDL_Event &);
};
