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
extern SDLDevice *g_sdlDevice;

void TextButton::drawEx(int nDstX, int nDstY, int nSrcX, int nSrcY, int nW, int nH) const
{
    const auto bgColor = colorf::ARGB2RGBA(m_color[getState()][1]);
    g_sdlDevice->fillRectangle(bgColor, nDstX, nDstY, nW, nH);

    const auto frameLineColor = colorf::ARGB2RGBA(m_frameLineColor[getState()]);
    g_sdlDevice->drawWidthRectangle(frameLineColor, m_frameLineWidth, nDstX, nDstY, nW, nH);

    int nLBX0 = (w() - m_label.w()) / 2;
    int nLBY0 = (h() - m_label.h()) / 2;

    int nLBW0 = m_label.w();
    int nLBH0 = m_label.h();

    int nLBX = nLBX0;
    int nLBY = nLBY0;

    int nLBW = nLBW0;
    int nLBH = nLBH0;

    if(mathf::rectangleOverlapRegion(nSrcX, nSrcY, nW, nH, &nLBX, &nLBY, &nLBW, &nLBH)){
        m_label.drawEx(nDstX + (nLBX - nSrcX) + offX(), nDstY + (nLBY - nSrcY) + offY(), nLBX - nLBX0, nLBY - nLBY0, nLBW, nLBH);
    }
}

void TextButton::setText(const char *format, ...)
{
    std::string text;
    str_format(format, text);

    m_label.setText(u8"%s", text.c_str());
    m_w = (std::max<int>)(m_w, m_label.w());
    m_h = (std::max<int>)(m_h, m_label.h());
}
