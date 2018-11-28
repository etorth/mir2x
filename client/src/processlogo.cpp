/*
 * =====================================================================================
 *
 *       Filename: processlogo.cpp
 *        Created: 08/13/2015 12:15:38
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

#include "client.hpp"
#include "sdldevice.hpp"
#include "pngtexdbn.hpp"
#include "processlogo.hpp"

void ProcessLogo::ProcessEvent(const SDL_Event &rstEvent)
{
    switch(rstEvent.type){
        case SDL_KEYDOWN:
            {
                switch(rstEvent.key.keysym.sym){
                    case SDLK_SPACE:
                    case SDLK_ESCAPE:
                        {
                            extern Client *g_Client;
                            g_Client->RequestProcess(PROCESSID_SYRC);
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

void ProcessLogo::Update(double fDTime)
{
    m_TotalTime += fDTime;
    if(m_TotalTime >= m_FullTime){
        extern Client *g_Client;
        g_Client->RequestProcess(PROCESSID_SYRC);
    }
}

void ProcessLogo::Draw()
{
    extern SDLDevice *g_SDLDevice;
    extern PNGTexDBN *g_ProgUseDBN;

    g_SDLDevice->ClearScreen();

    if(auto pTexture = g_ProgUseDBN->Retrieve(0X00000000)){
        auto bColor = (Uint8)(std::lround(255 * ColorRatio()));
        SDL_SetTextureColorMod(pTexture, bColor, bColor, bColor);

        auto nWindowW = g_SDLDevice->WindowW(false);
        auto nWindowH = g_SDLDevice->WindowH(false);
        g_SDLDevice->DrawTexture(pTexture, 0, 0, 0, 0, nWindowW, nWindowH);
    }

    g_SDLDevice->Present();
}

double ProcessLogo::ColorRatio()
{
    double fRatio = m_TotalTime / m_FullTime;
    if(fRatio < m_TimeR1){
        return fRatio / m_TimeR1;
    }else if(fRatio < m_TimeR1 + m_TimeR2){
        return 1.0;
    }else{
        return 1.0 - (fRatio - m_TimeR1 - m_TimeR2) / (1.0 - m_TimeR1 - m_TimeR2);
    }
}
