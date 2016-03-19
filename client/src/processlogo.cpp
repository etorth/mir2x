/*
 * =====================================================================================
 *
 *       Filename: processlogo.cpp
 *        Created: 8/13/2015 12:15:38 AM
 *  Last Modified: 03/19/2016 03:08:14
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

#include "game.hpp"
#include "sdldevice.hpp"
#include "pngtexdbn.hpp"
#include "processlogo.hpp"

ProcessLogo::ProcessLogo()
    : Process()
    , m_FullMS(5.0)
    , m_TimeR1(0.3)
    , m_TimeR2(0.3)
{}

ProcessLogo::~ProcessLogo()
{
}

void ProcessLogo::ProcessEvent(const SDL_Event &rstEvent)
{
    if(rstEvent.type == SDL_KEYDOWN && rstEvent.key.keysym.sym == SDLK_ESCAPE){
        extern Game *g_Game;
        g_Game->SwitchProcess(PROCESSID_LOGO, PROCESSID_SYRC);
    }
}

void ProcessLogo::Update(double fDMS)
{
    m_TotalTime += fDMS;
    if(m_TotalTime >= m_FullMS){
        extern Game *g_Game;
        g_Game->SwitchProcess(PROCESSID_LOGO, PROCESSID_SYRC);
    }else{
        Uint8 bColor = std::lround(255 * Ratio());
        extern SDLDevice *g_SDLDevice;
        g_SDLDevice->SetColor(bColor, bColor, bColor, bColor);
    }
}

void ProcessLogo::Draw()
{
    extern SDLDevice *g_SDLDevice;
    extern PNGTexDBN *g_PNGTexDBN;

    g_SDLDevice->DrawTexture(g_PNGTexDBN->Retrieve(255, 0), 0, 0);
}

double ProcessLogo::Ratio()
{
    double fRatio = m_TotalTime / m_FullMS;
    if(fRatio < m_TimeR1){
        return fRatio / m_TimeR1;
    }else if(fRatio < m_TimeR1 + m_TimeR2){
        return 1.0;
    }else{
        return 1.0 - (fRatio - m_TimeR1 - m_TimeR2) / (1.0 - m_TimeR1 - m_TimeR2);
    }
}
