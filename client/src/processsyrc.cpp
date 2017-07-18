/*
 * =====================================================================================
 *
 *       Filename: processsyrc.cpp
 *        Created: 08/14/2015 02:47:49
 *  Last Modified: 07/18/2017 15:21:50
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

#include "log.hpp"
#include "game.hpp"
#include "sdldevice.hpp"
#include "pngtexdbn.hpp"
#include "tokenboard.hpp"
#include "processsyrc.hpp"

ProcessSyrc::ProcessSyrc()
	: Process()
    , m_Ratio(0)
    , m_Info(100, 100, "Connecting...")
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
            {
                break;
            }
    }
}

void ProcessSyrc::Update(double fDeltaMS)
{
    if(m_Ratio >= 100){
        extern Game *g_Game;
        g_Game->SwitchProcess(PROCESSID_SYRC, PROCESSID_LOGIN);
    }

    m_Ratio += (fDeltaMS > 0.0 ? 1 : 0);
}

void ProcessSyrc::Draw()
{
    extern SDLDevice *g_SDLDevice;
    extern PNGTexDBN *g_ProgUseDBN;

    auto pTexture = g_ProgUseDBN->Retrieve(0X00000002);
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
    g_SDLDevice->DrawTexture(g_ProgUseDBN->Retrieve(0X00000001), 0, 0);

    int nInfoX = (g_SDLDevice->WindowW(false) - m_Info.W()) / 2;
    int nInfoY = 528 + (nH - m_Info.H()) / 2;

    m_Info.DrawEx(nInfoX, nInfoY, 0, 0, m_Info.W(), m_Info.H());
    g_SDLDevice->Present();
}
