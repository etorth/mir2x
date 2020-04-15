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

extern Log *g_Log;
extern SDLDevice *g_SDLDevice;

void TextButton::drawEx(int nDstX, int nDstY, int nSrcX, int nSrcY, int nW, int nH)
{
    extern SDLDevice *g_SDLDevice;
    g_SDLDevice->PushColor(colorf::RGBA2Color(colorf::ARGB2RGBA(m_Color[State()][1])));
    g_SDLDevice->FillRectangle(nDstX, nDstY, nW, nH);
    g_SDLDevice->PopColor();

    g_SDLDevice->PushColor(colorf::RGBA2Color(colorf::ARGB2RGBA(m_FrameLineColor[State()])));
    g_SDLDevice->DrawRectangle(m_FrameLineWidth, nDstX, nDstY, nW, nH);
    g_SDLDevice->PopColor();

    int nLBX0 = (w() - m_Label.w()) / 2;
    int nLBY0 = (h() - m_Label.h()) / 2;

    int nLBW0 = m_Label.w();
    int nLBH0 = m_Label.h();

    int nLBX = nLBX0;
    int nLBY = nLBY0;

    int nLBW = nLBW0;
    int nLBH = nLBH0;

    if(mathf::rectangleOverlapRegion(nSrcX, nSrcY, nW, nH, &nLBX, &nLBY, &nLBW, &nLBH)){
        m_Label.SetFontColor(m_Color[State()][0]);
        m_Label.drawEx(nDstX + (nLBX - nSrcX) + OffX(), nDstY + (nLBY - nSrcY) + OffY(), nLBX - nLBX0, nLBY - nLBY0, nLBW, nLBH);
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
        g_Log->addLog(LOGTYPE_WARNING, "%s", szError.c_str());
    }

    m_Label.setText("%s", szText.c_str());
    m_w = (std::max<int>)(m_w, m_Label.w());
    m_h = (std::max<int>)(m_h, m_Label.h());
}
