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

#include "mathfunc.hpp"
#include "sdldevice.hpp"
#include "textbutton.hpp"
#include "tritexbutton.hpp"

void TextButton::DrawEx(int nDstX, int nDstY, int nSrcX, int nSrcY, int nW, int nH)
{
    extern SDLDevice *g_SDLDevice;
    g_SDLDevice->PushColor(m_Color[State()][1]);
    g_SDLDevice->FillRectangle(nDstX, nDstY, nW, nH);
    g_SDLDevice->PopColor();

    g_SDLDevice->PushColor(m_FrameLineColor[State()]);
    g_SDLDevice->DrawRectangle(m_FrameLineWidth, nDstX, nDstY, nW, nH);
    g_SDLDevice->PopColor();

    int nLBX0 = (W() - m_Label.W()) / 2;
    int nLBY0 = (H() - m_Label.H()) / 2;

    int nLBW0 = m_Label.W();
    int nLBH0 = m_Label.H();

    int nLBX = nLBX0;
    int nLBY = nLBY0;

    int nLBW = nLBW0;
    int nLBH = nLBH0;

    if(RectangleOverlapRegion(nSrcX, nSrcY, nW, nH, &nLBX, &nLBY, &nLBW, &nLBH)){
        m_Label.SetColor(m_Color[State()][0]);
        m_Label.DrawEx(nDstX + (nLBX - nSrcX) + OffX(), nDstY + (nLBY - nSrcY) + OffY(), nLBX - nLBX0, nLBY - nLBY0, nLBW, nLBH);
    }
}
