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
#include "totype.hpp"
#include "autoalpha.hpp"
#include "flwrapper.hpp"
#include "mainwindow.hpp"
#include "previewwindow.hpp"
#include "wilimagepackage.hpp"

extern WilImagePackage *g_WilPackage;
extern MainWindow      *g_MainWindow;

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

bool PreviewWindow::LoadImage()
{
    if(!g_WilPackage->setIndex(g_MainWindow->SelectedImageIndex())){
        return false;
    }

    auto nW = g_WilPackage->currImageInfo()->width;
    auto nH = g_WilPackage->currImageInfo()->height;

    m_Buf.resize(0);
    m_Buf.resize(nW * nH);
    g_WilPackage->decode(m_Buf.data(), 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);

    if(g_MainWindow->AutoAlphaEnabled()){
        CalcPixelAutoAlpha(m_Buf.data(), m_Buf.size());
    }

    if(g_MainWindow->ShadowRemovalEnabled()){
        CalcShadowRemovalAlpha(m_Buf.data(), nW, nH, 0X80000000);
    }

    // Fl_RGB_Image won't copy the RGBA buffer
    // caller need to maintain m_Buf when m_Image is still used

    m_Image.reset(Fl_RGB_Image((uchar *)(m_Buf.data()), nW, nH, 4).copy());
    m_ImageIndex.emplace(g_MainWindow->SelectedImageIndex());

    size_t nWinH = (std::max<int>)(((std::min<int>)((to_d(nH * 1.5)), to_d(nH + 40))), (int)200);
    size_t nWinW = (std::max<int>)(((std::min<int>)((to_d(nW * 1.5)), to_d(nW + 40))), (int)200);

    // resize the window
    // don't call this function in the ::draw()
    size(nWinW, nWinH);
    copy_label(std::to_string(m_ImageIndex.value()).c_str());

    return true;
}
