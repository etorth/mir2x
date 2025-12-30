#include "colorf.hpp"
#include "totype.hpp"
#include "bevent.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "alphaonbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

AlphaOnButton::AlphaOnButton(AlphaOnButton::InitArgs args)
    : TrigfxButton
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .onOverIn  = std::move(args.onOverIn),
          .onOverOut = std::move(args.onOverOut),

          .onClick   = std::move(args.onClick),
          .onTrigger = std::move(args.onTrigger),

          .onClickDone = args.triggerOnDone,
          .parent = args.parent,
      }}

    , m_onOffX(args.onOffX)
    , m_onOffY(args.onOffY)
    , m_onRadius(args.onRadius)

    , m_down
      {{
          .texLoadFunc = [texID = std::move(args.downTexID), this] -> SDL_Texture *
          {
              return g_progUseDB->retrieve(Widget::evalU32(texID, this));
          },
      }}

    , m_on
      {{
          .w = std::nullopt,
          .h = std::nullopt,

          .childList
          {
              {
                  .widget = new ImageBoard
                  {{

                      .w = [this]{ return m_down.h(); }, // downTexID has blank alpha area on right side, use h() as w()
                      .h = [this]{ return m_down.h(); },

                      .texLoadFunc = [this] -> SDL_Texture *
                      {
                          return g_sdlDevice->getCover(m_onRadius, 360);
                      },

                      .modColor = std::move(args.modColor),
                  }},

                  .x = m_onOffX,
                  .y = m_onOffY,

                  .autoDelete = true,
              },
          },
      }}

    , m_off
      {{
          .w = [this]{ return m_on.w(); },
          .h = [this]{ return m_on.h(); },
      }}
{
    setGfxList({&m_off, &m_on, &m_down});
}
