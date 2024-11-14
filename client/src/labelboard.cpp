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
    setSize(m_tpset.px() + m_tpset.pw(), m_tpset.py() + m_tpset.ph());
}
