#include "totype.hpp"
#include "strf.hpp"
#include "xmlf.hpp"
#include "xmltypeset.hpp"
#include "labelboard.hpp"

LabelBoard::LabelBoard(LabelBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = 0, // use override w() and h()
          .h = 0,

          .attrs
          {
              .type
              {
                  .canSetSize = false,
              },
              .inst = std::move(args.attrs),
          },

          .parent = std::move(args.parent),
      }}

    , m_tpset
      {
          0,
          LALIGN_LEFT,
          false,
          args.font.id,
          args.font.size,
          args.font.style,
          std::move(args.font.color),
      }
{
    setText(u8"%s", args.label ? args.label : u8"");
}

void LabelBoard::setText(const char8_t *format, ...)
{
    std::u8string text;
    str_format(format, text);
    loadXML(xmlf::toParString("%s", text.empty() ? "" : to_cstr(text)).c_str());
}

void LabelBoard::loadXML(const char *xmlString)
{
    // use the fallback values of m_tpset
    // don't need to specify the font/size/style info here

    m_tpset.loadXML(xmlString);
}

void LabelBoard::setFont(uint8_t argFont)
{
    m_tpset.setFont(argFont);
    m_tpset.updateGfx();
}

void LabelBoard::setFontSize(uint8_t argFontSize)
{
    m_tpset.setFontSize(argFontSize);
    m_tpset.updateGfx();
}

void LabelBoard::setFontStyle(uint8_t argFontStyle)
{
    m_tpset.setFontStyle(argFontStyle);
    m_tpset.updateGfx();
}

void LabelBoard::setFontColor(Widget::VarU32 argFontColor)
{
    m_tpset.setFontColor(std::move(argFontColor));
}

void LabelBoard::setImageMaskColor(Widget::VarU32 argColor)
{
    m_tpset.setImageMaskColor(std::move(argColor));
}

void LabelBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }
    m_tpset.draw(m);
}
