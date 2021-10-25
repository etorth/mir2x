/*
 * =====================================================================================
 *
 *       Filename: notifyboard.cpp
 *        Created: 03/22/2020 16:45:16
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include "strf.hpp"
#include "totype.hpp"
#include "xmltypeset.hpp"
#include "mathf.hpp"
#include "notifyboard.hpp"

void NotifyBoard::addLog(const char8_t * format, ...)
{
    std::u8string text;
    str_format(format, text);

    while((m_maxEntryCount > 0) && (m_boardList.size() >= m_maxEntryCount)){
        m_boardList.pop_front();
    }

    m_boardList.emplace_back();
    m_boardList.back().typeset =std::make_unique<XMLTypeset>(m_lineW, LALIGN_LEFT, false, m_font, m_fontSize, m_fontStyle, m_fontColor);

    const auto xmlString = str_printf("<par>%s</par>", to_cstr(text));
    m_boardList.back().typeset->loadXML(xmlString.c_str());
    updateSize();
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
        updateSize();
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

        if(!mathf::ROICrop(
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

void NotifyBoard::updateSize()
{
    m_w = 0;
    m_h = 0;

    for(const auto &tp: m_boardList){
        m_h += tp.typeset->ph();
        if(m_lineW > 0){
            m_w = m_lineW;
        }
        else{
            m_w = std::max<int>(m_w, tp.typeset->pw());
        }
    }
}
