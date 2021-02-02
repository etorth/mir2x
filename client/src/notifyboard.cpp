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
#include "colorf.hpp"
#include "notifyboard.hpp"

void NotifyBoard::addLog(const char8_t * format, ...)
{
    std::u8string text;
    str_format(format, text);

    if(m_boardList.size() < 5){
        m_boardList.push_back(std::make_shared<XMLTypeset>(m_lineW, LALIGN_LEFT, false, m_font, m_fontSize, m_fontStyle, m_fontColor));
    }
    else{
        m_boardList.push_back(m_boardList.front());
        m_boardList.pop_front();
    }

    const auto xmlString = str_printf("<par>%s</par>", to_cstr(text));
    m_boardList.back()->loadXML(xmlString.c_str());

    m_w = m_lineW;
    m_h = 0;

    for(const auto &ptr: m_boardList){
        m_h += ptr->ph();
    }
}

void NotifyBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    int startX = 0;
    int startY = 0;

    for(const auto &ptr: m_boardList){
        const auto p = ptr.get();

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

int NotifyBoard::pw()
{
    int maxW = 0;
    for(const auto &ptr: m_boardList){
        maxW = std::max<int>(maxW, ptr->pw());
    }
    return maxW;
}
