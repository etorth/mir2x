#include <algorithm>
#include "colorf.hpp"
#include "progusedb.hpp"
#include "stallbar.hpp"

extern ProgUseDB *g_progUseDB;

StallBar::StallBar(StallBar::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::nullopt,
          .h = std::nullopt,

          .attrs
          {
              .inst
              {
                  .moveOnFocus = false,
              },
          },

          .parent = std::move(args.parent),
      }}

    , m_name
      {{
          .textFunc = std::move(args.nameFunc),
          .font
          {
              .color = colorf::YELLOW_A255,
          },
      }}

    , m_bg
      {{
          .w = [this]{ return nameSpaceWidth() + 10; },
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000235); },
      }}

    , m_frameImage
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000234); },
      }}

    , m_frame
      {{
          .getter = &m_frameImage,
          .vr
          {
              10, // x
               0, // y
              80, // w
              20, // h
          },

          .resize
          {
              [this]{ return nameSpaceWidth(); },
              20,
          },
      }}

    , m_gfx
      {{
          .childList
          {
              {&m_bg   , false},
              {&m_name , false},
              {&m_frame, false},
          },
      }}

    , m_button
      {{
          .gfxList
          {
              &m_gfx,
              &m_gfx,
              &m_gfx,
          },

          .onTrigger = [onClick = std::move(args.onClick)](Widget *, int)
          {
              if(onClick){
                  onClick();
              }
          },

          .parent{this},
      }}
{
    m_bg.setColor([this] -> uint32_t
    {
        switch(m_button.getState()){
            case BEVENT_ON  : return colorf::RGBA(255, 255, 255, 255);
            case BEVENT_DOWN: return colorf::RGBA(150, 150, 150, 255);
            default         : return colorf::RGBA(200, 200, 200, 255);
        }
    });
}
