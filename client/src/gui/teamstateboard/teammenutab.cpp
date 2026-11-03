#include <utility>
#include "totype.hpp"
#include "colorf.hpp"
#include "sdldevice.hpp"
#include "teammenutab.hpp"

extern SDLDevice *g_sdlDevice;
TeamMenuTab::TeamMenuTab(TeamMenuTab::InitArgs args)
    : TrigfxButton
      {{
          .dir = std::move(args.dir),
          .x = std::move(args.x),
          .y = std::move(args.y),

          .onTrigger = [onClick = std::move(args.onClick)](Widget *, int)
          {
              if(onClick){
                  onClick();
              }
          },

          .attrs = std::move(args.attrs),
          .parent = std::move(args.parent),
      }}

    , m_selected(std::move(args.selected))

    , m_gfx
      {{
          .w = args.w,
          .h = args.h,
      }}

    , m_background
      {{
          .w = args.w,
          .h = args.h,

          .drawFunc = [this](const Widget *self, int drawDstX, int drawDstY)
          {
              if(selected()){
                  g_sdlDevice->fillRectangle(colorf::RGBA(  0, 80, 255, 96), drawDstX, drawDstY, self->w(), self->h(), 5);
              }
              else if(getState() != BEVENT_OFF){
                  g_sdlDevice->fillRectangle(colorf::RGBA(255, 255, 255, 48), drawDstX, drawDstY, self->w(), self->h(), 5);
              }
          },

          .parent{&m_gfx},
      }}

    , m_label
      {{
          .dir = DIR_NONE,
          .x = args.w / 2,
          .y = args.h / 2,

          .textFunc = std::move(args.label),
          .font
          {
              .color = [this]
              {
                  return selected() ? colorf::YELLOW_A255 : colorf::WHITE_A255;
              },
          },

          .parent{&m_gfx},
      }}
{
    setGfxFunc([this]{ return &m_gfx; });
}
