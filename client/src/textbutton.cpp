/*
 * =====================================================================================
 *
 *       Filename: textbutton.cpp
 *        Created: 03/16/2017 15:04:17
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
#include "strf.hpp"
#include "mathf.hpp"
#include "sdldevice.hpp"
#include "textbutton.hpp"
#include "tritexbutton.hpp"

extern Log *g_log;
extern SDLDevice *g_SDLDevice;

void TextButton::drawEx(int nDstX, int nDstY, int nSrcX, int nSrcY, int nW, int nH)
{
    const auto bgColor = colorf::ARGB2RGBA(m_color[state()][1]);
    g_SDLDevice->fillRectangle(bgColor, nDstX, nDstY, nW, nH);

    const auto frameLineColor = colorf::ARGB2RGBA(m_frameLineColor[state()]);
    g_SDLDevice->drawWidthRectangle(frameLineColor, m_frameLineWidth, nDstX, nDstY, nW, nH);

    int nLBX0 = (w() - m_label.w()) / 2;
    int nLBY0 = (h() - m_label.h()) / 2;

    int nLBW0 = m_label.w();
    int nLBH0 = m_label.h();

    int nLBX = nLBX0;
    int nLBY = nLBY0;

    int nLBW = nLBW0;
    int nLBH = nLBH0;

    if(mathf::rectangleOverlapRegion(nSrcX, nSrcY, nW, nH, &nLBX, &nLBY, &nLBW, &nLBH)){
        m_label.setFontColor(m_color[state()][0]);
        m_label.drawEx(nDstX + (nLBX - nSrcX) + offX(), nDstY + (nLBY - nSrcY) + offY(), nLBX - nLBX0, nLBY - nLBY0, nLBW, nLBH);
    }
}

void TextButton::FormatText(const char *szFormatText, ...)
{
    std::string szText;
    std::string szError;
    {
        va_list ap;
        va_start(ap, szFormatText);

        try{
            szText = str_vprintf(szFormatText, ap);
        }catch(const std::exception &e){
            szText = "INTERNAL_ERROR";
            szError = str_printf("Exception caught in TextButton::FormatText(\"%s\", ...): %s", szFormatText, e.what());
        }

        va_end(ap);
    }

    if(!szError.empty()){
        g_log->addLog(LOGTYPE_WARNING, "%s", szError.c_str());
    }

    m_label.setText(u8"%s", szText.c_str());
    m_w = (std::max<int>)(m_w, m_label.w());
    m_h = (std::max<int>)(m_h, m_label.h());
}
