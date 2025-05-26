#include "strf.hpp"
#include "mathf.hpp"
#include "totype.hpp"
#include "xmltypeset.hpp"
#include "notifyboard.hpp"

NotifyBoard::NotifyBoard(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        int argLineW,

        uint8_t  argDefaultFont,
        uint8_t  argDefaultFontSize,
        uint8_t  argDefaultFontStyle,
        uint32_t argDefaultFontColor,

        uint64_t argShowTime,
        size_t   argMaxEntryCount,

        Widget *argWidgetPtr,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          [this](const Widget *)
          {
              if(m_lineW > 0){
                  return m_lineW;
              }

              int maxW = 0;
              for(const auto &tp: m_boardList){
                  maxW = std::max<int>(maxW, tp.typeset->pw());
              }
              return maxW;
          },

          [this](const Widget *)
          {
              int sumH = 0;
              for(const auto &tp: m_boardList){
                  sumH += tp.typeset->ph();
              }
              return sumH;
          },

          {},

          argWidgetPtr,
          argAutoDelete,
      }

    , m_lineW(argLineW)
    , m_font(argDefaultFont)
    , m_fontSize(argDefaultFontSize)
    , m_fontStyle(argDefaultFontStyle)
    , m_fontColor(argDefaultFontColor)
    , m_showTime(argShowTime)
    , m_maxEntryCount(argMaxEntryCount)
{}

void NotifyBoard::addLog(const char8_t * format, ...)
{
    std::u8string text;
    str_format(format, text);

    while((m_maxEntryCount > 0) && (m_boardList.size() >= m_maxEntryCount)){
        m_boardList.pop_front();
    }

    m_boardList.emplace_back();
    m_boardList.back().typeset = std::make_unique<XMLTypeset>(m_lineW, LALIGN_LEFT, false, m_font, m_fontSize, m_fontStyle, m_fontColor);

    const auto xmlString = xmlf::toParString("%s", text.empty() ? "" : to_cstr(text));
    m_boardList.back().typeset->loadXML(xmlString.c_str());
}

void NotifyBoard::update(double)
{
    if(m_showTime > 0){
        while(!m_boardList.empty()){
            if(m_boardList.front().timer.diff_msec() >= m_showTime){
                m_boardList.pop_front();
            }
            else{
                break;
            }
        }
    }
}

void NotifyBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    int startX = 0;
    int startY = 0;

    for(const auto &tp: m_boardList){
        const auto p = tp.typeset.get();

        int srcXCrop = srcX;
        int srcYCrop = srcY;
        int dstXCrop = dstX;
        int dstYCrop = dstY;
        int srcWCrop = srcW;
        int srcHCrop = srcH;

        if(!mathf::cropROI(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    w(),
                    h(),

                    0, startY, p->pw(), p->ph(), 0, 0, -1, -1)){
            break;
        }

        p->drawEx(dstXCrop, dstYCrop, srcXCrop - startX, srcYCrop - startY, srcWCrop, srcHCrop);
        startY += p->ph();
    }
}

int NotifyBoard::pw() const
{
    int maxW = 0;
    for(const auto &tp: m_boardList){
        maxW = std::max<int>(maxW, tp.typeset->pw());
    }
    return maxW;
}
