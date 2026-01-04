#include <utility>
#include "fflerror.hpp"
#include "itempair.hpp"

ItemPair::ItemPair(ItemPair::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = [v=args.v, flex=args.flex, first=fflcheck(args.first.widget), second=fflcheck(args.second.widget), this]
          {
              if(v){
                  return std::max<int>(first->w(), second->w());
              }
              else if(flex.has_value()){
                  return Widget::evalSize(flex.value(), this);
              }
              else{
                  return first->w() + second->w();
              }
          },

          .h = [v=args.v, flex=args.flex, first=fflcheck(args.first.widget), second=fflcheck(args.second.widget), this]
          {
              if(!v){
                  return std::max<int>(first->h(), second->h());
              }
              else if(flex.has_value()){
                  return Widget::evalSize(flex.value(), this);
              }
              else{
                  return first->h() + second->h();
              }
          },

          .childList
          {
              {
                  .widget = args.first.widget,
                  .dir = [v = args.v, align = args.align]
                  {
                      switch(align){
                          case ItemAlign::UPLEFT   : return v ? DIR_UPLEFT : DIR_LEFTUP  ;
                          case ItemAlign::CENTER   : return v ? DIR_UP     : DIR_LEFT    ;
                          case ItemAlign::DOWNRIGHT: return v ? DIR_UPRIGHT: DIR_LEFTDOWN;
                      }
                      std::unreachable();
                  }(),

                  .x = [v = args.v, align = args.align, this]
                  {
                      switch(align){
                          case ItemAlign::UPLEFT   : return v ?       0 : 0;
                          case ItemAlign::CENTER   : return v ? w() / 2 : 0;
                          case ItemAlign::DOWNRIGHT: return v ? w() - 1 : 0;
                      }
                      std::unreachable();
                  },

                  .y = [v = args.v, align = args.align, this]
                  {
                      switch(align){
                          case ItemAlign::UPLEFT   : return v ? 0 :       0;
                          case ItemAlign::CENTER   : return v ? 0 : h() / 2;
                          case ItemAlign::DOWNRIGHT: return v ? 0 : h() - 1;
                      }
                      std::unreachable();
                  },

                  .autoDelete = args.first.autoDelete,
              },

              {
                  .widget = args.second.widget,
                  .dir = [v = args.v, align = args.align]
                  {
                      switch(align){
                          case ItemAlign::UPLEFT   : return v ? DIR_DOWNLEFT  : DIR_RIGHTUP  ;
                          case ItemAlign::CENTER   : return v ? DIR_DOWN      : DIR_RIGHT    ;
                          case ItemAlign::DOWNRIGHT: return v ? DIR_DOWNRIGHT : DIR_RIGHTDOWN;
                      }
                      std::unreachable();
                  }(),

                  .x = [v = args.v, align = args.align, this]
                  {
                      switch(align){
                          case ItemAlign::UPLEFT   : return v ?       0 : w() - 1;
                          case ItemAlign::CENTER   : return v ? w() / 2 : w() - 1;
                          case ItemAlign::DOWNRIGHT: return v ? w() - 1 : w() - 1;
                      }
                      std::unreachable();
                  },

                  .y = [v = args.v, align = args.align, this]
                  {
                      switch(align){
                          case ItemAlign::UPLEFT   : return v ? h() - 1 :       0;
                          case ItemAlign::CENTER   : return v ? h() - 1 : h() / 2;
                          case ItemAlign::DOWNRIGHT: return v ? h() - 1 : h() - 1;
                      }
                      std::unreachable();
                  },

                  .autoDelete = args.second.autoDelete,
              }
          },

          .attrs
          {
              .type {.setSize  = false},
              .inst {.moveOnFocus = false},
          },

          .parent = std::move(args.parent),
      }}
{}
