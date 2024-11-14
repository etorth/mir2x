#include "log.hpp"
#include "totype.hpp"
#include "strf.hpp"
#include "xmlf.hpp"
#include "xmltypeset.hpp"
#include "labelboard.hpp"

extern Log *g_log;

LabelBoard::LabelBoard(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        const char8_t *argContent,
        uint8_t        argFont,
        uint8_t        argFontSize,
        uint8_t        argFontStyle,
        uint32_t       argFontColor,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          0,
          0,

          {},

          argParent,
          argAutoDelete,
      }

    , m_tpset
      {
          0,
          LALIGN_LEFT,
          false,
          argFont,
          argFontSize,
          argFontStyle,
          argFontColor,
      }
{
    setText(u8"%s", argContent ? argContent : u8"");
    setSize([this](const Widget *){ return m_tpset.px() + m_tpset.pw(); },
            [this](const Widget *){ return m_tpset.py() + m_tpset.ph(); });
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
    m_tpset.build();
}

void LabelBoard::setFontSize(uint8_t argFontSize)
{
    m_tpset.setFontSize(argFontSize);
    m_tpset.build();
}

void LabelBoard::setFontStyle(uint8_t argFontStyle)
{
    m_tpset.setFontStyle(argFontStyle);
    m_tpset.build();
}

void LabelBoard::setFontColor(uint32_t argFontColor)
{
    m_tpset.setFontColor(argFontColor);
}

void LabelBoard::setImageMaskColor(uint32_t argColor)
{
    m_tpset.setImageMaskColor(argColor);
}
