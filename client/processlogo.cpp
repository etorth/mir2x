/*
 * =====================================================================================
 *
 *       Filename: processlogo.cpp
 *        Created: 8/13/2015 12:15:38 AM
 *  Last Modified: 09/03/2015 6:26:45 AM
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


#include <algorithm>
#include "processlogo.hpp"
#include "texturemanager.hpp"
#include "devicemanager.hpp"

ProcessLogo::ProcessLogo(Game *pGame)
	: Process(Process::PROCESSID_LOGO, pGame)
    , m_TextureLogo()
    , m_StartTime(0)
    , m_FullMS(8000)
    , m_StartPartRatio(0.33)
    , m_StayPartRatio(0.33)
{}

ProcessLogo::~ProcessLogo()
{}

void ProcessLogo::Enter()
{
    Process::Enter();
    m_FrameCount = 0;
    if(!m_TextureLogo){
        m_TextureLogo = GetTextureManager()->RetrieveTexture(63, 0, 0);
        SDL_SetTextureBlendMode(m_TextureLogo, SDL_BLENDMODE_MOD);
    }
}

void ProcessLogo::Exit()
{
    Process::Exit();
    SDL_SetRenderDrawColor(GetDeviceManager()->GetRenderer(), 0, 0, 0, 0);
}

double ProcessLogo::Ratio()
{
    double fRatio = m_FrameCount * 1000.0 / m_FPS / m_FullMS;
    if(fRatio < m_StartPartRatio){
        return fRatio / m_StartPartRatio;
    }
    if(fRatio < m_StartPartRatio + m_StayPartRatio){
        return 1.0;
    }

    return (std::min)(1.0, (1.0 - fRatio) / (1.0 - m_StartPartRatio - m_StayPartRatio));
}

void ProcessLogo::Update()
{
	m_FrameCount++;
    Uint8  bColor = std::lround(Ratio() * 255);
    SDL_SetRenderDrawColor(GetDeviceManager()->GetRenderer(), bColor, bColor, bColor, bColor);
    if(m_FrameCount > m_FullMS * m_FPS / 1000){
        m_NextProcessID = Process::PROCESSID_SYRC;
    }
}

void ProcessLogo::Draw()
{
    SDL_RenderCopy(GetDeviceManager()->GetRenderer(), m_TextureLogo, nullptr, nullptr);
}

void ProcessLogo::HandleEvent(SDL_Event *pEvent)
{
    if(pEvent){
        switch(pEvent->type){
            case SDL_KEYDOWN:
                {
                    if(pEvent->key.keysym.sym == SDLK_ESCAPE){
                        m_NextProcessID = Process::PROCESSID_SYRC;
                    }
                    break;
                }
            default:
                break;
        }
    }
}
