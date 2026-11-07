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

    , m_button
      {{
          .dir = DIR_NONE,
          .x = [this]{ return w() / 2; },
          .y = [this]{ return h() / 2; },

          .texIDList
          {
              .off  {0X00000235},
              .on   {0X00000235},
              .down {0X00000235},
          },

          .onTrigger = [onClick = std::move(args.onClick)](Widget *, int)
          {
              if(onClick){
                  onClick();
              }
          },

          .modColor = [this] -> uint32_t
          {
              switch(m_button.getState()){
                  case BEVENT_ON  : return colorf::RGBA(255, 255, 255, 255);
                  case BEVENT_DOWN: return colorf::RGBA(150, 150, 150, 255);
                  default         : return colorf::RGBA(200, 200, 200, 255);
              }
          },

          .parent{this},
      }}

    , m_name
      {{
          .dir = DIR_NONE,
          .x = [this]{ return w() / 2; },
          .y = [this]{ return h() / 2; },

          .textFunc = std::move(args.nameFunc),
          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::YELLOW_A255,
          },

          .parent{this},
      }}

    , m_frame
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000234); },
          .parent{this},
      }}
{
    setSize([this]{ return m_frame.w(); },
            [this]{ return m_frame.h(); });
}
