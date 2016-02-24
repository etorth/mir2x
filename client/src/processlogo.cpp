/*
 * =====================================================================================
 *
 *       Filename: processlogo.cpp
 *        Created: 8/13/2015 12:15:38 AM
 *  Last Modified: 02/24/2016 02:48:06
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

static double Ratio(Uint32 nCurrentMS, Uint32 nFullMS, double fStep1R, double fStep2R)
{
    double fRatio = nCurrentMS * 1.0 / nFullMS;
    if(fRatio < fStep1R){
        return fRatio / fStep1R;
    }else if(fRatio < fStep1R + fStep2R){
        return 1.0;
    }else{
        return 1.0 - (fRatio -fStep1R - fStep2R) / (1.0 - fStep1R - fStep2R);
    }
}

void ProcessLogo::ProcessEvent(SDL_Event *pEvent)
{
    if(true
            && pEvent
            && pEvent->type == SDL_KEYDOWN
            && pEvent->key.keysym.sym == SDLK_ESCAPE
      ){
        m_Game->SwitchProcess(PROCESSID_LOGO, PROCESSID_SYRC);
    }
}

void ProcessLogo::Update()
{
    Uint32 nMaxMS = 5000;
    Uint32 nTmpMS = SDL_GetTicks() - m_StartMS;

    if(nTmpMS >= 5000){
        m_Game->SwitchProcess(PROCESSID_LOGO, PROCESSID_SYRC);
    }else{
        double fRatio = Ratio(nTmpMS, 5000, 0.3, 0.4);
        Uint8 bColor = std::lround(255 * fRatio);
        m_Game->SetRenderDrawColor(bColor, bColor, bColor, bColor);
    }
}

void ProcessLogo::Reset()
{
    m_StateStartTick = SDL_GetTicks();
}

void ProcessLogo::Draw()
{
    m_Game->DrawImage(255, 0);
}
