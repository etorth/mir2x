/*
 * =====================================================================================
 *
 *       Filename: processlogo.hpp
 *        Created: 8/13/2015 12:07:39 AM
 *  Last Modified: 09/03/2015 3:15:29 AM
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

#include <SDL.h>
#include "process.hpp"
#include "texturemanager.hpp"

class ProcessLogo: public Process
{
    private:
        Uint32  m_FullMS;
        double  m_StartPartRatio;
        double  m_StayPartRatio;
    private:
        SDL_Texture    *m_TextureLogo;
        Uint32          m_StartTime;
        int             m_FrameCount;
        int             m_ProcessID;
    public:
        ProcessLogo(Game *);
        virtual ~ProcessLogo();
    public:
        double Ratio();
    public:
        void Enter();
        void Exit();
        void Update();
        void Draw();
        void HandleEvent(SDL_Event *);
};
