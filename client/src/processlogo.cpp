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
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processlogo.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_SDLDevice;

void ProcessLogo::processEvent(const SDL_Event &event)
{
    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_SPACE:
                    case SDLK_ESCAPE:
                        {
                            g_client->RequestProcess(PROCESSID_SYRC);
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

void ProcessLogo::update(double fDTime)
{
    m_totalTime += fDTime;
    if(m_totalTime >= m_fullTime){
        g_client->RequestProcess(PROCESSID_SYRC);
    }
}

void ProcessLogo::draw()
{
    SDLDevice::RenderNewFrame newFrame;

    if(auto pTexture = g_progUseDB->Retrieve(0X00000000)){
        auto bColor = (Uint8)(std::lround(255 * colorRatio()));
        SDL_SetTextureColorMod(pTexture, bColor, bColor, bColor);

        const auto nWindowW = g_SDLDevice->getRendererWidth();
        const auto nWindowH = g_SDLDevice->getRendererHeight();
        g_SDLDevice->DrawTexture(pTexture, 0, 0, 0, 0, nWindowW, nWindowH);
    }
}

double ProcessLogo::colorRatio()
{
    const double fRatio = m_totalTime / m_fullTime;
    if(fRatio < m_timeR1){
        return fRatio / m_timeR1;
    }

    else if(fRatio < m_timeR1 + m_timeR2){
        return 1.0;
    }

    else{
        return 1.0 - (fRatio - m_timeR1 - m_timeR2) / (1.0 - m_timeR1 - m_timeR2);
    }
}
