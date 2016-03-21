/*
 * =====================================================================================
 *
 *       Filename: processsyrc.cpp
 *        Created: 08/14/2015 2:47:49 PM
 *  Last Modified: 03/20/2016 19:37:51
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
#include "pngtexdbn.hpp"
#include "processsyrc.hpp"
#include "tokenboard.hpp"
#include "sdldevice.hpp"

ProcessSyrc::ProcessSyrc()
	: Process()
    , m_Ratio(0)
    , m_Info(0, 14, 0, {0XFF, 0XFF, 0XFF, 0XFF}, "Connecting...")
{
}

ProcessSyrc::~ProcessSyrc()
{}

void ProcessSyrc::ProcessEvent(const SDL_Event &rstEvent)
{
    switch(rstEvent.type){
        case SDL_KEYDOWN:
            {
                if(rstEvent.key.keysym.sym == SDLK_ESCAPE){
                    extern Game *g_Game;
                    g_Game->SwitchProcess(PROCESSID_LOGO, PROCESSID_LOGIN);
                }
                break;
            }
        default:
            break;
    }
}

void ProcessSyrc::Update(double fDeltaMS)
{
    m_Ratio += (fDeltaMS > 10.0);

    if(m_Ratio >= 100){
        m_Ratio = m_Ratio - 100;
    }
}

void ProcessSyrc::Draw()
{
    extern PNGTexDBN  *g_PNGTexDBN;
    extern SDLDevice  *g_SDLDevice;

    auto pTexture = g_PNGTexDBN->Retrieve(255, 2);
    int nW, nH;

    SDL_QueryTexture(pTexture, nullptr, nullptr, &nW, &nH);

    g_SDLDevice->ClearScreen();
    g_SDLDevice->DrawTexture(pTexture,
            112,  // dst x
            528,  // dst y
            0,    // src x
            0,    // src y
            std::lround(nW * (m_Ratio / 100.0)), // src w
            nH);  // src h
    g_SDLDevice->DrawTexture(g_PNGTexDBN->Retrieve(255, 1), 0, 0);

    extern SDLDevice  *g_SDLDevice;
    m_Info.Draw((g_SDLDevice->WindowW() - m_Info.W()) / 2, 528 + (nH - m_Info.H()) / 2);
    g_SDLDevice->Present();
}
