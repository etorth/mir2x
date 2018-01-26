/*
 * =====================================================================================
 *
 *       Filename: processlogo.hpp
 *        Created: 08/13/2015 12:07:39
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
        double m_TimeR1;
        double m_TimeR2;

    private:
        double m_FullTime;
        double m_TotalTime;

    public:
        ProcessLogo()
            : Process()
            , m_TimeR1(0.3)
            , m_TimeR2(0.3)
            , m_FullTime(5000.0)
            , m_TotalTime(0.0)
        {}

    public:
        virtual ~ProcessLogo() = default;

    public:
        int ID() const
        {
            return PROCESSID_LOGO;
        }

    public:
        void Draw();
        void Update(double);
        void ProcessEvent(const SDL_Event &);

    private:
        double ColorRatio();
};
