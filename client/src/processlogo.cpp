/*
 * =====================================================================================
 *
 *       Filename: processlogo.cpp
 *        Created: 8/13/2015 12:15:38 AM
 *  Last Modified: 03/20/2016 19:42:52
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
    , m_FullMS(5000.0)
    , m_TimeR1(0.3)
    , m_TimeR2(0.3)
    , m_TotalTime(0.0)
{}

ProcessLogo::~ProcessLogo()
{
}

void ProcessLogo::ProcessEvent(const SDL_Event &rstEvent)
{
    switch(rstEvent.type){
        case SDL_KEYDOWN:
            {
                switch(rstEvent.key.keysym.sym){
                    case SDLK_SPACE:
                    case SDLK_ESCAPE:
                        {
                            extern Game *g_Game;
                            g_Game->SwitchProcess(PROCESSID_LOGO, PROCESSID_SYRC);
                        }
                        break;
                    default:
                        break;
                }
                break;
            }
        default:
            break;
    }
}

void ProcessLogo::Update(double fDMS)
{
    m_TotalTime += fDMS;
    if(m_TotalTime >= m_FullMS){
        extern Game *g_Game;
        g_Game->SwitchProcess(PROCESSID_LOGO, PROCESSID_SYRC);
    }
}

void ProcessLogo::Draw()
{
    extern SDLDevice *g_SDLDevice;
    extern PNGTexDBN *g_PNGTexDBN;

    g_SDLDevice->ClearScreen();
    // TODO
    // there is a bug here
    // between set some parameters to the texture and
    // take that parameters into effect, if there is any
    // release, then these effect of the parameters won't show
    //
    // so any operation on texture should be done and consumed ASAP

    Uint8 bColor = std::lround(255 * Ratio());
    auto pTexture = g_PNGTexDBN->Retrieve(255, 0);
    SDL_SetTextureColorMod(pTexture, bColor, bColor, bColor);

    g_SDLDevice->DrawTexture(pTexture, 0, 0);
    g_SDLDevice->Present();
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
