#pragma once
#include <functional>
#include "widget.hpp"
#include "textboard.hpp"

class TextShadowBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarInt shadowX = 0;
            Widget::VarInt shadowY = 0;

            Widget::VarStrFunc textFunc {};
            Widget::FontConfig font {};

            Widget::VarU32       shadowColor = colorf::BLACK + colorf::A_SHF(128);
            Widget::VarBlendMode blendMode   = SDL_BLENDMODE_BLEND;

            Widget::WADPair parent {};
        };

    private:
        TextBoard m_textShadow;
        TextBoard m_text;

    public:
        TextShadowBoard(TextShadowBoard::InitArgs args)
            : Widget
              {{
                  .dir = std::move(args.dir),

                  .x = std::move(args.x),
                  .y = std::move(args.y),

                  .w = std::nullopt,
                  .h = std::nullopt,

                  .parent = std::move(args.parent),
              }}

            , m_textShadow
              {{
                  .x = std::move(args.shadowX),
                  .y = std::move(args.shadowY),

                  .textFunc = args.textFunc, // do NOT move
                  .font
                  {
                      .id    = args.font.id,
                      .size  = args.font.size,
                      .style = args.font.style,
                      .color = std::move(args.shadowColor),
                  },

                  .blendMode = args.blendMode,
                  .parent{this},
              }}

            , m_text
              {{
                  .textFunc = std::move(args.textFunc),
                  .font     = std::move(args.font),

                  .blendMode = std::move(args.blendMode),
                  .parent{this},
              }}
        {}
};
