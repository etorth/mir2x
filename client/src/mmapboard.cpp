/*
 * =====================================================================================
 *
 *       Filename: mmapboard.cpp
 *        Created: 10/08/2017 19:22:30
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
#include "mmapboard.hpp"
#include "processrun.hpp"

extern Client *g_client;
extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

MMapBoard::MMapBoard(ProcessRun *runPtr, Widget *parent, bool autoDelete)
    : Widget
      {
          0,
          0,
          0,
          0,

          parent,
          autoDelete,
      }
    , m_processRun(runPtr)
{}

void MMapBoard::drawEx(int, int, int, int, int, int) const
{
    switch(m_status){
        case MMAP_OFF:
            {
                return;
            }
        case MMAP_ON:
        case MMAP_EXTENDED:
            {
                if(auto texPtr = g_progUseDB->Retrieve(m_status == MMAP_ON ? 0X09000000 : 0X09000001); texPtr){
                    const auto [rdrW, rdrH] = g_sdlDevice->getRendererSize();
                    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
                    if(!m_alphaOn){
                        g_sdlDevice->fillRectangle(colorf::BLACK + 255, rdrW - texW, 0, texW, texH);
                    }
                    g_sdlDevice->drawTexture(texPtr, rdrW - texW, 0);
                }
                return;
            }
        case MMAP_FULLSCREEN:
            {
                return;
            }
        default:
            {
                throw bad_reach();
            }
    }
}

bool MMapBoard::processEvent(const SDL_Event &, bool)
{
    return false;
}
