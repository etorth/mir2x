/*
 * =====================================================================================
 *
 *       Filename: quickaccessbutton.cpp
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
#include "bevent.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "quickaccessbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_SDLDevice;

QuickAccessButton::QuickAccessButton(const std::function<void()> &fnOnClick, Widget *pwidget, bool autoDelete)
    : ButtonBase
      {
          148,
          2,
          0,
          0,

          nullptr,
          fnOnClick,

          0,
          0,
          0,
          0,

          true,
          pwidget,
          autoDelete,
      }
{
    auto texPtr = g_progUseDB->Retrieve(m_texID);
    if(!texPtr){
        throw fflerror("no valid texture for quick access button: texID = %llu", to_llu(m_texID));
    }
    std::tie(m_w, m_h) = SDLDevice::getTextureSize(texPtr);
}

void QuickAccessButton::drawEx(int dstX, int dstY, int, int, int, int)
{
    switch(m_state){
        case BEVENT_ON:
            {
                constexpr int radius = 9;
                auto texPtr = g_SDLDevice->getCover(radius);
                if(!texPtr){
                    throw fflerror("can't get round cover: radius = %d", radius);
                }

                const auto [texW, texH] = SDLDevice::getTextureSize(texPtr);
                const int texDrawH = texH * 2 / 3;

                SDL_SetTextureAlphaMod(texPtr, 80);
                g_SDLDevice->DrawTexture(texPtr, dstX + 3, dstY, 0, texDrawH, texW, texDrawH);
                break;
            }
        case BEVENT_DOWN:
            {
                if(auto texPtr = g_progUseDB->Retrieve(m_texID)){
                    g_SDLDevice->DrawTexture(texPtr, dstX, dstY);
                }
                break;
            }
        default:
            {
                break;
            }
    }
}
