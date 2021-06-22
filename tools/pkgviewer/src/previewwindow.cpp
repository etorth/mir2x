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
#include "imgf.hpp"
#include "totype.hpp"
#include "alphaf.hpp"
#include "colorf.hpp"
#include "flwrapper.hpp"
#include "mainwindow.hpp"
#include "previewwindow.hpp"
#include "wilimagepackage.hpp"

extern WilImagePackage *g_WilPackage;
extern MainWindow      *g_MainWindow;

void PreviewWindow::draw()
{
    Fl_Double_Window::draw();
    if(g_MainWindow->clearBackgroundEnabled()){
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
    if(!g_WilPackage->setIndex(g_MainWindow->selectedImageIndex())){
        return false;
    }

    const auto nW = g_WilPackage->currImageInfo()->width;
    const auto nH = g_WilPackage->currImageInfo()->height;

    m_imageBuf.clear();
    m_imageBuf.resize(nW * nH, 0X00000000);

    const auto layer = g_WilPackage->decode(false, g_MainWindow->removeShadowMosaicEnabled(), g_MainWindow->autoAlphaEnabled());

    if(layer[0] && g_MainWindow->layerIndexEnabled(0)){
        imgf::blendImageBuffer(m_imageBuf.data(), nW, nH, layer[0], nW, nH, 0, 0);
    }

    if(layer[1] && g_MainWindow->layerIndexEnabled(1)){
        imgf::blendImageBuffer(m_imageBuf.data(), nW, nH, layer[1], nW, nH, 0, 0);
    }

    if(layer[2] && g_MainWindow->layerIndexEnabled(2)){
        imgf::blendImageBuffer(m_imageBuf.data(), nW, nH, layer[2], nW, nH, 0, 0);
    }

    // Fl_RGB_Image won't copy the RGBA buffer
    // caller need to maintain m_Buf when m_Image is still used

    m_Image.reset(Fl_RGB_Image((uchar *)(m_imageBuf.data()), nW, nH, 4).copy());
    m_ImageIndex.emplace(g_MainWindow->selectedImageIndex());

    size_t nWinH = (std::max<int>)(((std::min<int>)((to_d(nH * 1.5)), to_d(nH + 40))), (int)200);
    size_t nWinW = (std::max<int>)(((std::min<int>)((to_d(nW * 1.5)), to_d(nW + 40))), (int)200);

    // resize the window
    // don't call this function in the ::draw()
    size(nWinW, nWinH);
    copy_label(std::to_string(m_ImageIndex.value()).c_str());

    return true;
}
