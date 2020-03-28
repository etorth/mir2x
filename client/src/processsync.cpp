/*
 * =====================================================================================
 *
 *       Filename: processsync.cpp
 *        Created: 08/14/2015 02:47:49
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
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "tokenboard.hpp"
#include "processsync.hpp"

extern Client *g_Client;
extern PNGTexDB *g_ProgUseDB;
extern SDLDevice *g_SDLDevice;

ProcessSync::ProcessSync()
	: Process()
    , m_Ratio(0)
    , m_ProcessBarInfo(0, 0, "Connecting...", 1, 10, 0)
{} 
void ProcessSync::processEvent(const SDL_Event &event)
{
    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(event.key.keysym.sym == SDLK_ESCAPE){
                    g_Client->RequestProcess(PROCESSID_LOGIN);
                }
                break;
            }
        default:
            {
                break;
            }
    }
}

void ProcessSync::Update(double fDeltaMS)
{
    if(m_Ratio >= 100){
        g_Client->RequestProcess(PROCESSID_LOGIN);
        return;
    }

    m_Ratio += (fDeltaMS > 0.0 ? 1 : 0);
}

void ProcessSync::Draw()
{
    auto pTexture = g_ProgUseDB->Retrieve(0X00000002);
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
    g_SDLDevice->DrawTexture(g_ProgUseDB->Retrieve(0X00000001), 0, 0);

    int nInfoX = (g_SDLDevice->WindowW(false) - m_ProcessBarInfo.W()) / 2;
    int nInfoY = 528 + (nH - m_ProcessBarInfo.H()) / 2;

    m_ProcessBarInfo.drawEx(nInfoX, nInfoY, 0, 0, m_ProcessBarInfo.W(), m_ProcessBarInfo.H());
    g_SDLDevice->Present();
}
