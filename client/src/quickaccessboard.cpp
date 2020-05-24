/*
 * =====================================================================================
 *
 *       Filename: quickaccessboard.cpp
 *        Created: 03/28/2020 05:43:45
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

#include <tuple>
#include "toll.hpp"
#include "pngtexdb.hpp"
#include "sysconst.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "quickaccessboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_SDLDevice;

QuickAccessBoard::QuickAccessBoard(int x, int y, ProcessRun *proc)
    : Widget
      {
          x,
          y,
      }
     
    , m_proc(proc)
    , m_buttonClose
      {
          263,
          32,
          {SYS_TEXNIL, 0X00000061, 0X00000062},

          nullptr,
          [this]()
          {
              show(false);
          },

          0,
          0,
          0,
          0,

          true,
          this
      }
{
    auto texPtr = g_progUseDB->Retrieve(m_texID);
    if(!texPtr){
        throw fflerror("no valid quick access board texture: texID = %llu", toLLU(m_texID));
    }

    show(false);
    std::tie(m_w, m_h) = SDLDevice::getTextureSize(texPtr);
}

void QuickAccessBoard::drawEx(int dstX, int dstY, int, int, int, int)
{
    auto texPtr = g_progUseDB->Retrieve(m_texID);
    if(!texPtr){
        throw fflerror("no valid quick access board texture: texID = %llu", toLLU(m_texID));
    }

    g_SDLDevice->DrawTexture(texPtr, dstX, dstY);
    m_buttonClose.drawEx(dstX + m_buttonClose.dx(), dstY + m_buttonClose.dy(), 0, 0, m_buttonClose.w(), m_buttonClose.h());
}

bool QuickAccessBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return false;
    }

    if(!show()){
        return false;
    }

    if(m_buttonClose.processEvent(event, valid)){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEMOTION:
            {
                if(in(event.motion.x, event.motion.y) && (event.motion.state & SDL_BUTTON_LMASK)){
                    const auto [rendererW, rendererH] = g_SDLDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, x() + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, y() + event.motion.yrel));

                    moveBy(newX - x(), newY - y());
                    return true;
                }
                return false;
            }
        default:
            {
                return false;
            }
    }
}
