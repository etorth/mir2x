/*
 * =====================================================================================
 *
 *       Filename: processlogo.hpp
 *        Created: 8/13/2015 12:07:39 AM
 *  Last Modified: 03/17/2016 01:10:30
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

class ProcessLogo: public Process
{
    private:
        double  m_FullMS;
        double  m_TimeR1;
        double  m_TimeR2;

    public:
        ProcessLogo();
        virtual ~ProcessLogo();

    public:
        int ID()
        {
            return PROCESSID_LOGO;
        }

    private:
        double Ratio();

    public:
        void Update(double);
        void Draw();
        void ProcessEvent(const SDL_Event &);
};
