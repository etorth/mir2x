/*
 * =====================================================================================
 *
 *       Filename: processlogo.cpp
 *        Created: 8/13/2015 12:15:38 AM
 *  Last Modified: 01/23/2016 04:29:24
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

void Game::ProcessEventOnLogo(SDL_Event *pEvent)
{
    if(true
            && pEvent
            && pEvent->type == SDL_KEYDOWN
            && pEvent->key.keysym.sym == SDLK_ESCAPE
      ){
        SwitchProcess(PROCESSID_LOGO, PROCESSID_SYRC);
    }
}

void Game::UpdateOnLogo()
{
    Uint32 nTmpMS = SDL_GetTicks();
    if(nTmpMS - m_StateStartTick >= m_StateLogoMaxTick){
        SwitchProcess(PROCESSID_LOGO, PROCESSID_SYRC);
    }else{
        double fRatio = Ratio(nTmpMS - m_StateStartTick, m_StateLogoMaxTick, 0.3, 0.4);
        Uint8 bColor = std::lround(255 * fRatio);
        SDL_SetRenderDrawColor(m_Renderer, bColor, bColor, bColor, bColor);
    }
}

void Game::DrawOnLogo()
{
    SDL_RenderCopy(m_Renderer,m_GUITexManager.Retrieve(0), nullptr, nullptr);
}
