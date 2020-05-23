/*
 * =====================================================================================
 *
 *       Filename: alphaonbutton.cpp
 *        Created: 08/26/2016 13:20:23
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

#include "toll.hpp"
#include "bevent.hpp"
#include "sdldevice.hpp"
#include "alphaonbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_SDLDevice;

AlphaOnButton::AlphaOnButton(
        int x,
        int y,

        int onOffX,
        int onOffY,
        int onRadius,

        uint32_t onColor,
        uint32_t downTexID,

        const std::function<void()> &fnOnOver,
        const std::function<void()> &fnOnClick,

        bool    triggerOnDone,
        Widget *pwidget,
        bool    autoDelete)
    : ButtonBase
      { 
          x,
          y,
          0,
          0,

          fnOnOver,
          fnOnClick,

          0,
          0,
          0,
          0,

          triggerOnDone,
          pwidget,
          autoDelete,
      }
    , m_onColor(onColor)
    , m_texID(downTexID)
    , m_onOffX(onOffX)
    , m_onOffY(onOffY)
    , m_onRadius(onRadius)
{
    if(auto texPtr = g_progUseDB->Retrieve(m_texID)){
        const auto [texW, texH] = SDLDevice::getTextureSize(texPtr);
        m_w = texW;
        m_h = texH;
    }
    throw fflerror("can't load down texture: %llu", toLLU(m_texID));
}

void AlphaOnButton::drawEx(int dstX, int dstY, int, int, int, int)
{
    switch(m_state){
        case BEVENT_ON:
            {
                auto texPtr = g_SDLDevice->getCover(m_onRadius);
                if(!texPtr){
                    throw fflerror("can't get round cover: radius = %llu", toLLU(m_onRadius));
                }

                g_SDLDevice->DrawTexture(texPtr, dstX + m_onOffX, dstY + m_onOffY);
                break;
            }
        case BEVENT_DOWN:
            {
                auto texPtr = g_progUseDB->Retrieve(m_texID);
                if(!texPtr){
                    throw fflerror("can't get down texture: texID = %llu", toLLU(m_texID));
                }

                g_SDLDevice->DrawTexture(texPtr, dstX, dstY);
                break;
            }
        default:
            {
                break;
            }
    }
}
