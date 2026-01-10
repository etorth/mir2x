#include "fontexdb.hpp"
#include "textboard.hpp"

extern FontexDB *g_fontexDB;
TextBoard::TextBoard(TextBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),
          .w = {},
          .h = {},

          .attrs
          {
              .inst = std::move(args.attrs),
          },
          .parent = std::move(args.parent),
      }}

    , m_font(std::move(args.font))
    , m_textFunc(std::move(args.textFunc))

    , m_image
      {{
          .texLoadFunc = [this]() -> SDL_Texture *
          {
              if(const auto s = getVStr(); s.empty()){
                  return nullptr;
              }
              else{
                  return g_fontexDB->retrieve(m_font.id, m_font.size, m_font.style, s.c_str());
              }
          },

          .modColor = [this]
          {
              return Widget::evalU32(m_font.color, this);
          },

          .blendMode = std::move(args.blendMode),
          .parent{this},
      }}
{
    if(!g_fontexDB->hasFont(m_font.id)){
        throw fflerror("invalid font: %hhu", m_font.id);
    }
}

std::tuple<std::string, std::string> TextBoard::fontName() const
{
    return g_fontexDB->fontName(m_font.id);
}
