/*
 * =====================================================================================
 *
 *       Filename: previewwindow.cpp
 *        Created: 07/22/2015 03:16:57 AM
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

#include <string>
#include <cstring>
#include "flwrapper.hpp"
#include "platforms.hpp"
#include "mainwindow.hpp"
#include "previewwindow.hpp"
#include "wilimagepackage.hpp"

extern WilImagePackage  g_WilPackage;
extern MainWindow      *g_MainWindow;

static void CalcAutoAlpha(uint32_t *pData, size_t nDataLen, const std::string &szAutoAlphaMethod)
{
    if(!(pData && nDataLen)){
        return;
    }

    if(szAutoAlphaMethod.empty() || szAutoAlphaMethod == "None"){
        return;
    }

    enum
    {
        AUTOALPHA_NONE = 0,
        AUTOALPHA_HSL,
        AUTOALPHA_CMYK,
    };

    int nMethod = 0;
    if(szAutoAlphaMethod == "HSL"){
        nMethod = AUTOALPHA_HSL;
    }else if(szAutoAlphaMethod == "CMYK"){
        nMethod = AUTOALPHA_CMYK;
    }else{
        return;
    }

    auto fnGetAlpha = [nMethod](uint8_t r, uint8_t g, uint8_t b) -> double
    {
        switch(nMethod){
            case AUTOALPHA_HSL:
                {
                    constexpr double factor = 5.0;

                    double l = (0.30 * r + 0.59 * g + 0.11 * b) / 255.0;
                    double x = (l * 2.0 - 1.0) * factor;

                    return 1.0 / (1.0 + std::exp(-x));
                }
            case AUTOALPHA_CMYK:
                {
                    return std::max<double>({r / 255.0, g / 255.0, b / 255.0});
                }
            default:
                {
                    return 1.0;
                }
        }
    };

    for(size_t nIndex = 0; nIndex < nDataLen; ++nIndex){
        uint8_t a = ((pData[nIndex] & 0XFF000000) >> 24);

        if(a == 0){
            continue;
        }

        uint8_t r = ((pData[nIndex] & 0X00FF0000) >> 16);
        uint8_t g = ((pData[nIndex] & 0X0000FF00) >>  8);
        uint8_t b = ((pData[nIndex] & 0X000000FF) >>  0);

        uint32_t lum = std::lround(fnGetAlpha(r, g, b) * 255.0);

        pData[nIndex] &= 0X00FFFFFF;
        pData[nIndex] |= (lum << 24);
    }
}

void PreviewWindow::draw()
{
    Fl_Double_Window::draw();
    if(g_MainWindow->BlackBG()){
        fl_rectf(0, 0, w(), h(), 0, 0, 0);
    }

    if(!m_Image){
        return;
    }

    int nX = (w() - m_Image->w()) / 2;
    int nY = (h() - m_Image->h()) / 2;
    int nW = m_Image->w();
    int nH = m_Image->h();

    m_Image->draw(nX, nY, nW, nH);
    {
        fl_wrapper::enable_color stColor(FL_RED);
        fl_rect(nX, nY, nW, nH);
    }
}

void PreviewWindow::LoadImage()
{
    if(!g_WilPackage.SetIndex(g_MainWindow->SelectedImageIndex())){
        return;
    }

    if(!g_WilPackage.CurrentImageValid()){
        return;
    }

    auto nW = g_WilPackage.CurrentImageInfo().shWidth;
    auto nH = g_WilPackage.CurrentImageInfo().shHeight;

    m_Buf.resize(0);
    m_Buf.resize(nW * nH);

    g_WilPackage.Decode(m_Buf.data(), 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);

    if(auto szAutoAlphaMethod = g_MainWindow->GetAutoAlphaMethod(); szAutoAlphaMethod != "None"){
        CalcAutoAlpha(m_Buf.data(), m_Buf.size(), szAutoAlphaMethod);
    }

    m_Image = std::make_unique<Fl_RGB_Image>((uchar *)(m_Buf.data()), nW, nH, 4);
    m_ImageIndex.emplace(g_MainWindow->SelectedImageIndex());

    size_t nWinH = (std::max<int>)(((std::min<int>)(((int)(nH * 1.5)), (int)(nH + 40))), (int)200);
    size_t nWinW = (std::max<int>)(((std::min<int>)(((int)(nW * 1.5)), (int)(nW + 40))), (int)200);

    // resize the window
    // don't call this function in the ::draw()
    size(nWinW, nWinH);
    copy_label(std::to_string(m_ImageIndex.value()).c_str());
}
