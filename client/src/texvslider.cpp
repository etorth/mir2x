/*
 * =====================================================================================
 *
 *       Filename: texvslider.cpp
 *        Created: 08/12/2015 09:59:15
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

#include "colorf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "texvslider.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

TexVSlider::TexVSlider(dir8_t dir, int x, int y, int h, int paramIndex, const std::function<void(float)> &onChanged, Widget *parent, bool autoDelete)
    : Slider
      {
          dir,
          x,
          y,
          TexVSlider::getParam(paramIndex).w,
          h,

          onChanged,

          false,
          TexVSlider::getParam(paramIndex).sliderSize,
          TexVSlider::getParam(paramIndex).sliderSize,

          parent,
          autoDelete,
      }
    , m_sliderParamIndex(paramIndex)
{}

void TexVSlider::drawEx(int, int, int, int, int, int) const
{
    if(g_clientArgParser->debugSlider){
        g_sdlDevice->drawRectangle(colorf::GREEN + colorf::A_SHF(255), x(), y(), w(), h());
    }

    const auto [sliderX, sliderY, sliderW, sliderH] = getSliderRectangle();

    auto texPtr = g_progUseDB->retrieve(getSelfParam().texID);
    if(!texPtr){
        return;
    }

    const auto [valCenterX, valCenterY] = getValueCenter();
    g_sdlDevice->drawTexture(texPtr, valCenterX - getSelfParam().offX, valCenterY - getSelfParam().offY);

    const auto fnDrawCover = [valCenterX, valCenterY, this](uint32_t color)
    {
        const auto r = getSelfParam().sliderCover;
        if(auto texPtr = g_sdlDevice->getCover(r, 360)){
            SDL_SetTextureColorMod(texPtr, colorf::R(color), colorf::G(color), colorf::B(color));
            SDLDeviceHelper::EnableRenderBlendMode enableDrawBlendMode(SDL_BLENDMODE_BLEND);
            g_sdlDevice->drawTexture(texPtr, valCenterX - r, valCenterY - r);
        }
    };

    switch(m_sliderState){
        case BEVENT_ON:
            {
                fnDrawCover(colorf::BLUE + colorf::A_SHF(200));
                break;
            }
        case BEVENT_DOWN:
            {
                fnDrawCover(colorf::RED + colorf::A_SHF(200));
                break;
            }
        default:
            {
                break;
            }
    }

    if(g_clientArgParser->debugSlider){
        g_sdlDevice->drawRectangle(colorf::RED + colorf::A_SHF(255), sliderX, sliderY, sliderW, sliderH);
    }
}
