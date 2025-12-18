#include "colorf.hpp"
#include "totype.hpp"
#include "bevent.hpp"
#include "sdldevice.hpp"
#include "alphaonbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

AlphaOnButton::AlphaOnButton(AlphaOnButton::InitArgs args)
    : ButtonBase
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .onOverIn  = std::move(args.onOverIn),
          .onOverOut = std::move(args.onOverOut),

          .onClick = std::move(args.onClick),
          .onTrigger = std::move(args.onTrigger),

          .onClickDone = args.triggerOnDone,
          .parent = args.parent,
      }}

    , m_modColor(std::move(args.modColor))
    , m_downTexID(std::move(args.downTexID))

    , m_onOffX(args.onOffX)
    , m_onOffY(args.onOffY)
    , m_onRadius(args.onRadius)

    , m_on
      {{
          .x = m_onOffX,
          .y = m_onOffY,

          .w = [this](const Widget *){ return w(); },
          .h = [this](const Widget *){ return h(); },

          .texLoadFunc = [this](const Widget *) -> SDL_Texture *
          {
              return (getState() == BEVENT_ON) ? g_sdlDevice->getCover(m_onRadius, 360) : nullptr;
          },

          .modColor = m_modColor,
          .parent{this},
      }}

    , m_down
      {{
          .texLoadFunc = [this](const Widget *) -> SDL_Texture *
          {
              return (getState() == BEVENT_DOWN) ? g_progUseDB->retrieve(Widget::evalU32(m_downTexID, this)) : nullptr;
          },

          .parent{this},
      }}
{
    m_on  .setShow([this](const Widget *){ return getState() == BEVENT_ON  ; });
    m_down.setShow([this](const Widget *){ return getState() == BEVENT_DOWN; });

    setSize([this](const Widget *)
    {
        if(auto texPtr = g_progUseDB->retrieve(Widget::evalU32(m_downTexID, this))){
            return SDLDeviceHelper::getTextureWidth(texPtr);
        }
        return 0;
    },

    [this](const Widget *)
    {
        if(auto texPtr = g_progUseDB->retrieve(Widget::evalU32(m_downTexID, this))){
            return SDLDeviceHelper::getTextureHeight(texPtr);
        }
        return 0;
    });
}
