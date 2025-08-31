#include "colorf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "texslider.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

TexSlider::TexSlider(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget::VarSize argW,
        Widget::VarSize argH,

        bool argHSlider,
        int  argSliderIndex,

        std::function<void(float)> argOnChanged,

        Widget *argParent,
        bool    argAutoDelete)

    : Slider
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          std::move(argW),
          std::move(argH),

          argHSlider,
          getSliderTexInfo(argSliderIndex).w,
          getSliderTexInfo(argSliderIndex).h,

          std::move(argOnChanged),

          argParent,
          argAutoDelete,
      }

    , m_sliderTexInfo(getSliderTexInfo(argSliderIndex))
    , m_debugDraw
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          [this](const Widget *, int drawDstX, int drawDstY)
          {
              if(g_clientArgParser->debugSlider){
                  g_sdlDevice->drawRectangle(colorf::GREEN + colorf::A_SHF(255), drawDstX, drawDstY, w(), h());

                  const auto [valCenterX, valCenterY] = getValueCenter(drawDstX, drawDstY);
                  g_sdlDevice->drawLine(colorf::YELLOW + colorf::A_SHF(255), valCenterX, valCenterY, valCenterX - m_sliderTexInfo.offX, valCenterY - m_sliderTexInfo.offY);

                  const auto [sliderX, sliderY, sliderW, sliderH] = getSliderRectangle(drawDstX, drawDstY);
                  g_sdlDevice->drawRectangle(colorf::RED + colorf::A_SHF(255), sliderX, sliderY, sliderW, sliderH);
              }
          },

          this,
          false,
      }
{
    fflassert(w() > 0);
    fflassert(h() > 0);
    fflassert(m_sliderTexInfo.w > 0);
    fflassert(m_sliderTexInfo.h > 0);
    fflassert(m_sliderTexInfo.cover > 0);
    fflassert(g_progUseDB->retrieve(m_sliderTexInfo.texID), str_printf("%08X.PNG", m_sliderTexInfo.texID));
}

void TexSlider::drawEx(int dstX, int dstY, const Widget::ROIOpt &roi) const
{
    const auto [valCenterX, valCenterY] = getValueCenter();
    if(auto texPtr = g_progUseDB->retrieve(m_sliderTexInfo.texID)){
        const SDLDeviceHelper::EnableTextureModColor enableTexModColor(texPtr, active() ? colorf::RGBA(0XFF, 0XFF, 0XFF, 0XFF) : colorf::RGBA(0X80, 0X80, 0X80, 0XFF));
        g_sdlDevice->drawTexture(texPtr, valCenterX - m_sliderTexInfo.offX, valCenterY - m_sliderTexInfo.offY);
    }

    const auto fnDrawCover = [valCenterX, valCenterY, this](uint32_t color)
    {
        if(auto texPtr = g_sdlDevice->getCover(m_sliderTexInfo.cover, 360)){
            const SDLDeviceHelper::EnableTextureModColor enableTexModColor(texPtr, color);
            const SDLDeviceHelper::EnableRenderBlendMode enableDrawBlendMode(SDL_BLENDMODE_BLEND);
            g_sdlDevice->drawTexture(texPtr, valCenterX - m_sliderTexInfo.cover, valCenterY - m_sliderTexInfo.cover);
        }
    };

    switch(m_sliderState){
        case BEVENT_ON  : fnDrawCover(active() ? (colorf::BLUE + colorf::A_SHF(200)) : (colorf::WHITE + colorf::A_SHF(255))); break;
        case BEVENT_DOWN: fnDrawCover(active() ? (colorf::RED  + colorf::A_SHF(200)) : (colorf::WHITE + colorf::A_SHF(255))); break;
        default: break;
    }
}
