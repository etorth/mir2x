#include "colorf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "texslider.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

TexSlider::TexSlider(dir8_t argDir, int argX, int argY, int argW, int argH, bool hslider, int sliderIndex, std::function<void(float)> onChanged, Widget *parent, bool autoDelete)
    : Slider
      {
          argDir,
          argX,
          argY,
          argW,
          argH,

          hslider,
          getSliderTexInfo(sliderIndex).w,
          getSliderTexInfo(sliderIndex).h,

          std::move(onChanged),

          parent,
          autoDelete,
      }

    , m_sliderTexInfo(getSliderTexInfo(sliderIndex))
{
    fflassert(w() > 0);
    fflassert(h() > 0);
    fflassert(m_sliderTexInfo.w > 0);
    fflassert(m_sliderTexInfo.h > 0);
    fflassert(m_sliderTexInfo.cover > 0);
    fflassert(g_progUseDB->retrieve(m_sliderTexInfo.texID), str_printf("%08X.PNG", m_sliderTexInfo.texID));
}

void TexSlider::drawEx(int, int, int, int, int, int) const
{
    if(g_clientArgParser->debugSlider){
        g_sdlDevice->drawRectangle(colorf::GREEN + colorf::A_SHF(255), x(), y(), w(), h());
    }

    const auto [valCenterX, valCenterY] = getValueCenter();
    if(auto texPtr = g_progUseDB->retrieve(m_sliderTexInfo.texID)){
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
        g_sdlDevice->drawLine(colorf::YELLOW + colorf::A_SHF(255), valCenterX, valCenterY, valCenterX - m_sliderTexInfo.offX, valCenterY - m_sliderTexInfo.offY);
    }

    if(g_clientArgParser->debugSlider){
        const auto [sliderX, sliderY, sliderW, sliderH] = getSliderRectangle();
        g_sdlDevice->drawRectangle(colorf::RED + colorf::A_SHF(255), sliderX, sliderY, sliderW, sliderH);
    }
}
