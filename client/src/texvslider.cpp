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

#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "texvslider.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_SDLDevice;
extern ClientArgParser *g_clientArgParser;

TexVSlider::TexVSlider(int x, int y, int h, bool small, Widget *parent, bool autoDelete)
    : Slider
      {
          x,
          y,
          TexVSlider::getParm(small).w,
          h,

          false,
          TexVSlider::getParm(small).sliderSize,
          TexVSlider::getParm(small).sliderSize,

          parent,
          autoDelete,
      }
    , m_small(small)
{}

void TexVSlider::drawEx(int, int, int, int, int, int)
{
    if(g_clientArgParser->debugSlider){
        g_SDLDevice->DrawRectangle(colorf::GREEN + 255, x(), y(), w(), h());
    }

    const auto [sliderX, sliderY, sliderW, sliderH] = getSliderRectangle();

    auto texPtr = g_progUseDB->Retrieve(getSelfParm().texID);
    if(!texPtr){
        return;
    }

    const auto [valCenterX, valCenterY] = getValueCenter();
    g_SDLDevice->DrawTexture(texPtr, valCenterX - getSelfParm().offX, valCenterY - getSelfParm().offY);

    const auto fnDrawCover = [this](uint32_t color)
    {
        const auto r = getSelfParm().sliderCover;
        const auto [valCenterX, valCenterY] = getValueCenter();

        if(auto texPtr = g_SDLDevice->getCover(r)){
            SDL_SetTextureColorMod(texPtr, colorf::R(color), colorf::G(color), colorf::B(color));
            g_SDLDevice->DrawTexture(texPtr, valCenterX - r, valCenterY - r);
        }
    };

    switch(m_sliderState){
        case BEVENT_ON:
            {
                fnDrawCover(colorf::BLUE);
                break;
            }
        case BEVENT_DOWN:
            {
                fnDrawCover(colorf::RED);
                break;
            }
        default:
            {
                break;
            }
    }

    if(g_clientArgParser->debugSlider){
        g_SDLDevice->DrawRectangle(colorf::RED + 255, sliderX, sliderY, sliderW, sliderH);
    }
}
