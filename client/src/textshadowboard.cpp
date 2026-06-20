#include "textshadowboard.hpp"

TextShadowBoard::TextShadowBoard(TextShadowBoard::InitArgs args)
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
