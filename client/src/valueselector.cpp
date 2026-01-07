#include "sdldevice.hpp"
#include "valueselector.hpp"

extern SDLDevice *g_sdlDevice;

ValueSelector::ValueSelector(ValueSelector::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = 0,
          .h = std::move(args.h),

          .parent = std::move(args.parent),
      }}

    , m_input
      {{
          .w = std::move(args.input.w),
          .h = [this]{ return h(); },

          .enableIME = std::move(args.input.enableIME),

          .font   = std::move(args.input.font),
          .cursor = std::move(args.input.cursor),
      }}

    , m_up
      {{
          .w = std::move(args.button.w),
          .h = [this]{ return h() / 2; },

          .triangle
          {
              .dir = DIR_UP,

              .w = [](const Widget *self){ return self->w() / 2; },
              .h = [](const Widget *self){ return self->h() / 2; },

              .color = colorf::GREY_A255,
          },

          .frame
          {
              .color = colorf::GREY_A255,
          },

          .onTrigger = std::move(args.upTrigger),
      }}

    , m_down
      {{
          .w = [this]{ return       m_up.w(); },
          .h = [this]{ return h() - m_up.h(); },

          .triangle
          {
              .dir = DIR_DOWN,

              .w = [](const Widget *self){ return self->w() / 2; },
              .h = [](const Widget *self){ return self->h() / 2; },

              .color = colorf::GREY_A255,
          },

          .frame
          {
              .color = colorf::GREY_A255,
          },

          .onTrigger = std::move(args.downTrigger),
      }}

    , m_vflex
      {{
          .childList
          {
              {&m_up  , false},
              {&m_down, false},
          },
      }}

    , m_hflex
      {{
          .v = false,
          .headSpace = 2,

          .childList
          {
              {&m_input, false},
              {&m_vflex, false},
          },

          .parent{this},
      }}

    , m_frame
      {{
          .w = [this]{ return m_hflex.w(); },
          .h = [this]{ return m_hflex.h(); },

          .drawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
          {
              g_sdlDevice->drawRectangle(colorf::GREY_A255, dstDrawX, dstDrawY, self->w(), self->h());
          },

          .parent{this},
      }}
{
    setW([this]{ return m_hflex.w(); });
}
