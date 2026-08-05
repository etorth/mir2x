#include <utility>
#include "strf.hpp"
#include "colorf.hpp"
#include "auctionregisterprice.hpp"

AuctionRegisterPrice::AuctionRegisterPrice(AuctionRegisterPrice::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = 145,
          .h =  27,

          .attrs
          {
              .inst = std::move(args.attrs),
          },
          .parent = std::move(args.parent),
      }}

    , m_onClick(std::move(args.onClick))
    , m_text
      {{
          .dir = DIR_NONE,

          .x = [this]{ return w() / 2; },
          .y = [this]{ return h() / 2; },

          .textFunc = [this]() -> std::string
          {
              if(m_price){
                  return str_ksep(m_price);
              }
              return to_cstr(u8"点击设置价格");
          },

          .font
          {
              .color = [this]
              {
                  return m_price ? colorf::YELLOW_A255 : colorf::WHITE_A255;
              },
          },
          .parent{this},
      }}
{}

bool AuctionRegisterPrice::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(Widget::processEventDefault(event, valid && m_enabled, m)){
        return true;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if((event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) && (event.button.button == SDL_BUTTON_LEFT) && m.in(to_d(event.button.x), to_d(event.button.y))){
        if(m_enabled && m_onClick){
            m_onClick();
        }
        return consumeFocus(true);
    }
    return consumeFocus(false);
}
