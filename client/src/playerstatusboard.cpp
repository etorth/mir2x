/*
 * =====================================================================================
 *
 *       Filename: playerstatusboard.cpp
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

#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "inventoryboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_SDLDevice;

PlayerStatusBoard::PlayerStatusBoard(int argX, int argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          argX,
          argY,
          0,
          0,

          widgetPtr,
          autoDelete
      }

    , m_closeButton
      {
          288,
          13,
          {SYS_TEXNIL, 0X0000001C, 0X0000001D},

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
          this,
          false,
      }
    , m_processRun(runPtr)
{
    show(false);
    auto texPtr = g_progUseDB->Retrieve(0X06000000);
    if(!texPtr){
        throw fflerror("no valid player status board frame texture");
    }
    std::tie(m_w, m_h) = SDLDevice::getTextureSize(texPtr);
}

void PlayerStatusBoard::update(double)
{
}

void PlayerStatusBoard::drawEx(int dstX, int dstY, int, int, int, int)
{
    if(auto pTexture = g_progUseDB->Retrieve(0X06000000)){
        g_SDLDevice->drawTexture(pTexture, dstX, dstY);
    }
    m_closeButton.draw();
}

bool PlayerStatusBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return focusConsumer(this, false);
    }

    if(!show()){
        return focusConsumer(this, false);
    }

    if(m_closeButton.processEvent(event, valid)){
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
                    return focusConsumer(this, true);
                }
                return focusConsumer(this, false);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                switch(event.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            return focusConsumer(this, in(event.button.x, event.button.y));
                        }
                    default:
                        {
                            return focusConsumer(this, false);
                        }
                }
            }
        default:
            {
                return focusConsumer(this, false);
            }
    }
}
