#include "colorf.hpp"
#include "sysconst.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "tritexbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

TritexButton::TritexButton(TritexButton::InitArgs args)
    : ButtonBase
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .onOverIn  = std::move(args.onOverIn),
          .onOverOut = std::move(args.onOverOut),

          .onClick = std::move(args.onClick),
          .onTrigger = std::move(args.onTrigger),

          .seff = std::move(args.seff),

          .offXOnOver = args.offXOnOver,
          .offYOnOver = args.offYOnOver,

          .offXOnClick = args.offXOnClick,
          .offYOnClick = args.offYOnClick,

          .onClickDone = args.onClickDone,
          .radioMode = args.radioMode,

          .attrs = std::move(args.attrs),
          .parent = args.parent,
      }}

    , m_texIDList(args.texIDList)
    , m_alterColor(args.alterColor)
{
    const auto fnGetEdgeSize = [this](auto fn)
    {
        return [fn, this](const Widget *)
        {
            int result = 0;
            for(const int state: {0, 1, 2}){
                if(m_texIDList[state].has_value()){
                    if(auto texPtr = g_progUseDB->retrieve(m_texIDList[state].value())){
                        result = std::max<int>(result, fn(texPtr));
                    }
                }
            }

            // we allow buttons without any valid texture, in that case some extra work
            // can be done for special drawing
            return result;
        };
    };

    setSize(fnGetEdgeSize([](SDL_Texture *texPtr){ return SDLDeviceHelper::getTextureWidth (texPtr); }),
            fnGetEdgeSize([](SDL_Texture *texPtr){ return SDLDeviceHelper::getTextureHeight(texPtr); }));
}

void TritexButton::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    if(m_texIDList[getState()].has_value()){
        if(auto texPtr = g_progUseDB->retrieve(m_texIDList[getState()].value())){
            const int offX = m_offset[getState()][0];
            const int offY = m_offset[getState()][1];
            const auto modColor= [this]() -> uint32_t
            {
                if(!active()){
                    return colorf::RGBA(128, 128, 128, 255);
                }
                else if(m_alterColor && (getState() != BEVENT_OFF)){
                    return colorf::RGBA(255, 200, 255, 255);
                }
                else{
                    return colorf::RGBA(255, 255, 255, 255);
                }
            }();

            const SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, modColor);
            const SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, [this]()
            {
                if(m_blinkTime.has_value()){
                    const auto offTime = std::get<0>(m_blinkTime.value());
                    const auto  onTime = std::get<1>(m_blinkTime.value());

                    if(offTime == 0){
                        return SDL_BLENDMODE_ADD;
                    }
                    else if(onTime == 0){
                        return SDL_BLENDMODE_BLEND;
                    }
                    else{
                        if(std::fmod(m_accuBlinkTime, offTime + onTime) < offTime){
                            return SDL_BLENDMODE_BLEND;
                        }
                        else{
                            return SDL_BLENDMODE_ADD;
                        }
                    }
                }
                else{
                    return SDL_BLENDMODE_BLEND;
                }
            }());
            g_sdlDevice->drawTexture(texPtr, m.x + offX, m.y + offY, m.ro->x, m.ro->y, m.ro->w, m.ro->h); // TODO: need to crop m.ro-> region for offset
        }
    }
}
