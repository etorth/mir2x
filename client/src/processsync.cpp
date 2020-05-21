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

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_SDLDevice;

ProcessSync::ProcessSync()
	: Process()
    , m_ratio(0)
    , m_processBarInfo(0, 0, "Connecting...", 1, 10, 0)
{} 
void ProcessSync::processEvent(const SDL_Event &event)
{
    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(event.key.keysym.sym == SDLK_ESCAPE){
                    g_client->RequestProcess(PROCESSID_LOGIN);
                }
                break;
            }
        default:
            {
                break;
            }
    }
}

void ProcessSync::update(double fUpdateTime)
{
    if(m_ratio >= 100){
        g_client->RequestProcess(PROCESSID_LOGIN);
        return;
    }

    m_ratio += (fUpdateTime > 0.0 ? 1 : 0);
}

void ProcessSync::draw()
{
    SDLDevice::RenderNewFrame newFrame;
    auto pTexture = g_progUseDB->Retrieve(0X00000002);
    int nW, nH;

    SDL_QueryTexture(pTexture, nullptr, nullptr, &nW, &nH);

    g_SDLDevice->DrawTexture(pTexture,
            112,  // dst x
            528,  // dst y
            0,    // src x
            0,    // src y
            std::lround(nW * (m_ratio / 100.0)), // src w
            nH);  // src h
    g_SDLDevice->DrawTexture(g_progUseDB->Retrieve(0X00000001), 0, 0);

    const int nInfoX = (800 - m_processBarInfo.w()) / 2;
    const int nInfoY = 528 + (nH - m_processBarInfo.h()) / 2;

    m_processBarInfo.drawEx(nInfoX, nInfoY, 0, 0, m_processBarInfo.w(), m_processBarInfo.h());
}
