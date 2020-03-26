/*
 * =====================================================================================
 *
 *       Filename: debugboard.cpp
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

#include "log.hpp"
#include "strfunc.hpp"
#include "xmltypeset.hpp"
#include "mathfunc.hpp"
#include "colorfunc.hpp"
#include "debugboard.hpp"

extern Log *g_Log;

void DebugBoard::addLog(const char * formatString, ...)
{
    std::string text;
    bool error = false;
    {
        va_list ap;
        va_start(ap, formatString);

        try{
            text = str_vprintf(formatString, ap);
        }catch(const std::exception &e){
            error = true;
            text = str_printf("Exception caught in DebugBoard::addLog(\"%s\", ...): %s", formatString, e.what());
        }

        va_end(ap);
    }

    if(error){
        g_Log->AddLog(LOGTYPE_WARNING, "%s", text.c_str());
    }

    if(m_BoardList.size() < 5){
        m_BoardList.push_back(std::make_shared<XMLTypeset>(m_LineW, LALIGN_LEFT, true, 0, 0, m_DefaultFont, m_DefaultFontSize, m_DefaultFontStyle, m_DefaultFontColor));
    }
    else{
	m_BoardList.push_back(m_BoardList.front());
	m_BoardList.pop_front();
    }

    const auto xmlString = str_printf("<par>%s</par>", text.c_str());
    m_BoardList.back()->loadXML(xmlString.c_str());

    m_W = m_LineW;
    m_H = 0;

    for(const auto &ptr: m_BoardList){
        m_H += ptr->PH();
    }
}

void DebugBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int w, int h)
{
    int startX = 0;
    int startY = 0;

    for(const auto &ptr: m_BoardList){
        const auto p = ptr.get();

        int srcXCrop = srcX;
        int srcYCrop = srcY;
        int dstXCrop = dstX;
        int dstYCrop = dstY;
        int srcWCrop = w;
        int srcHCrop = h;

        if(!MathFunc::ROICrop(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    W(),
                    H(),

                    0, startY, p->PW(), p->PH(), 0, 0, -1, -1)){
            break;
        }

        p->drawEx(dstXCrop, dstYCrop, srcXCrop - startX, srcYCrop - startY, srcWCrop, srcHCrop);
        startY += p->PH();
    }
}

int DebugBoard::PW()
{
    int maxW = 0;
    for(const auto &ptr: m_BoardList){
        maxW = std::max<int>(maxW, ptr->PW());
    }
    return maxW;
}
