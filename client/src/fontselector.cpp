#include "colorf.hpp"
#include "fontexdb.hpp"
#include "sdldevice.hpp"
#include "textboard.hpp"
#include "fontselector.hpp"

extern FontexDB *g_fontexDB;
extern SDLDevice *g_sdlDevice;

FontSelector::FontSelector(FontSelector::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::nullopt,
          .h = std::nullopt,

          .parent = std::move(args.parent),
      }}

    , m_widget
      {{
          .label
          {
              .text = u8"控件",
          },

          .title
          {
              .text = u8"选择控件",
          },

          .itemList
          {
              {
                  .gfxWidget
                  {
                      .widget = new TextBoard
                      {{
                          .textFunc = "系统信息",
                      }},
                      .autoDelete = true,
                  },
              },

              {
                  .gfxWidget
                  {
                      .widget = new TextBoard
                      {{
                          .textFunc = "命令行",
                      }},
                      .autoDelete = true,
                  },
              },
          },

          .parent{this},
      }}

    , m_font
      {{
          .x = [this]{ return m_widget.fixedSize().w + FontSelector::GAP; },

          .label
          {
              .text = u8"字体",
          },

          .title
          {
              .text = u8"选择字体",
          },

          .onClick = [this](Widget *item)
          {
              const auto font = to_u8(std::any_cast<int>(item->data()));
              m_english.setFont(font);
              m_chinese.setFont(font);
          },

          .parent{this},
      }}

    , m_size
      {{
          .x = [this]{ return m_font.dx() + m_font.fixedSize().w + FontSelector::GAP; },
          .h = [this]{ return               m_font.fixedSize().h                    ; },

          .input
          {
              .w = 40,
          },

          .button
          {
              .w = [this]{ return m_font.fixedSize().h; },
          },

          .upTrigger = [this](int)
          {
              if(const auto val = m_size.getInt(); val.has_value()){
                  m_english.setFontSize(val.value());
                  m_chinese.setFontSize(val.value());
              }
          },

          .downTrigger = [this](int)
          {
              if(const auto val = m_size.getInt(); val.has_value()){
                  m_english.setFontSize(val.value());
                  m_chinese.setFontSize(val.value());
              }
          },

          .range = std::make_pair(5, 25),
          .parent{this},
      }}

    , m_english
      {{
          .lineWidth = FontSelector::LAYOUT_WIDTH,
          .initXML = "<layout><par>The quick brown fox jumps over the lazy dog.</par></layout>",
          .lineAlign = LALIGN_JUSTIFY,
      }}

    , m_chinese
      {{
          .lineWidth = FontSelector::LAYOUT_WIDTH,
          .initXML = "<layout><par>快速的棕色狐狸跳过了懒狗。</par></layout>",
          .lineAlign = LALIGN_JUSTIFY,
      }}

    , m_vflex
      {{
          .y = [this]
          {
              return std::max<int>(
              {
                  m_widget.fixedSize().h,
                  m_font  .fixedSize().h,
                  m_size              .h(),
              });
          },

          .fixed = FontSelector::LAYOUT_WIDTH,

          .headSpace = 3,
          .itemSpace = 2,
          .tailSpace = 3,

          .childList
          {
              {&m_english, false},
              {&m_chinese, false},
          },

          .parent{this},
      }}

    , m_vflexFrame
      {{
          .x = [this]{ return m_vflex.dx(); },
          .y = [this]{ return m_vflex.dy(); },

          .w = [this]{ return m_vflex.w(); },
          .h = [this]{ return m_vflex.h(); },

          .drawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
          {
              g_sdlDevice->drawRectangle(colorf::GREY_A255, dstDrawX, dstDrawY, self->w(), self->h());
          },

          .parent{this},
      }}
{
    size_t foundFont = 0;
    size_t fontCount = g_fontexDB->fontCount();

    for(uint8_t font = 0; foundFont < fontCount; ++font){
        if(g_fontexDB->hasFont(font)){
            const auto [name, style] = g_fontexDB->fontName(font);
            auto fontEntry = new TextBoard
            {{
                 .textFunc = name + " " + style,
                 .attrs
                 {
                     .data = to_d(font),
                 },
            }};

            m_font.addMenu({.gfxWidget{fontEntry, true}});
            foundFont++;
        }
    }

    if(const auto val = m_size.getInt(); val.has_value()){
        m_english.setFontSize(val.value());
        m_chinese.setFontSize(val.value());
    }
}
