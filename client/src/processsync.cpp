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
#include "processsync.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ProcessSync::ProcessSync()
	: Process()
    , m_ratio(0)
    , m_processBarInfo(DIR_UPLEFT, 0, 0, u8"Connecting...", 1, 10, 0)
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
    SDLDeviceHelper::RenderNewFrame newFrame;
    auto texPtr = g_progUseDB->retrieve(0X00000002);
    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);

    g_sdlDevice->drawTexture(texPtr,
            112,  // dst x
            528,  // dst y
            0,    // src x
            0,    // src y
            std::lround(texW * (m_ratio / 100.0)), // src w
            texH);  // src h
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X00000001), 0, 0);

    const int infoX = (800 - m_processBarInfo.w()) / 2;
    const int infoY = 528 + (texH - m_processBarInfo.h()) / 2;
    m_processBarInfo.drawEx(infoX, infoY, 0, 0, m_processBarInfo.w(), m_processBarInfo.h());
}
