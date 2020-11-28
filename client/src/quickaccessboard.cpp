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
#include "totype.hpp"
#include "pngtexdb.hpp"
#include "sysconst.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "quickaccessboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_SDLDevice;

QuickAccessBoard::QuickAccessBoard(int x, int y, ProcessRun *proc, Widget *pwidget, bool autoDelete)
    : Widget
      {
          x,
          y,
          0,
          0,
          pwidget,
          autoDelete,
      }
     
    , m_proc(proc)
    , m_buttonClose
      {
          263,
          32,
          {SYS_TEXNIL, 0X00000061, 0X00000062},

          nullptr,
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
        throw fflerror("no valid quick access board texture: texID = %llu", to_llu(m_texID));
    }

    show(false);
    std::tie(m_w, m_h) = SDLDevice::getTextureSize(texPtr);
}

void QuickAccessBoard::drawEx(int dstX, int dstY, int, int, int, int)
{
    auto texPtr = g_progUseDB->Retrieve(m_texID);
    if(!texPtr){
        throw fflerror("no valid quick access board texture: texID = %llu", to_llu(m_texID));
    }

    g_SDLDevice->drawTexture(texPtr, dstX, dstY);
    m_buttonClose.drawEx(dstX + m_buttonClose.dx(), dstY + m_buttonClose.dy(), 0, 0, m_buttonClose.w(), m_buttonClose.h());
}

bool QuickAccessBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        focus(false);
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
                if((event.motion.state & SDL_BUTTON_LMASK) && (in(event.motion.x, event.motion.y) || focus())){
                    const auto [rendererW, rendererH] = g_SDLDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, x() + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, y() + event.motion.yrel));
                    moveBy(newX - x(), newY - y());

                    focus(true);
                    return true;
                }

                focus(false);
                return false;
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                switch(event.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            if(in(event.button.x, event.button.y)){
                                focus(true);
                                return true;
                            }
                            else{
                                focus(false);
                                return false;
                            }
                        }
                    default:
                        {
                            focus(false);
                            return false;
                        }
                }
            }
        default:
            {
                return false;
            }
    }
}
