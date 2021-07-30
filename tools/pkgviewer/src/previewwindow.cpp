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

extern WilImagePackage *g_wilPackage;
extern MainWindow      *g_mainWindow;

void PreviewWindow::draw()
{
    Fl_Double_Window::draw();
    if(g_mainWindow->clearBackgroundEnabled()){
        fl_rectf(0, 0, w(), h(), 0, 0, 0);
    }

    if(!m_image){
        return;
    }

    const auto [nX, nY] = [this]() -> std::tuple<int, int>
    {
        if(g_mainWindow->offsetDrawEnabled()){
            return
            {
                100 + m_imageOffX,
                100 + m_imageOffY - m_image->h(),
            };
        }
        else{
            return
            {
                (w() - m_image->w()) / 2,
                (h() - m_image->h()) / 2,
            };
        }
    }();

    const int nW = m_image->w();
    const int nH = m_image->h();

    m_image->draw(nX, nY, nW, nH);
    {
        fl_wrapper::enable_color stColor(FL_RED);
        fl_rect(nX, nY, nW, nH);
    }

    if(g_mainWindow->showOffsetCrossEnabled()){
        const auto crossX = nX + m_imageOffX;
        const auto crossY = nY + m_imageOffY;
        {
            fl_wrapper::enable_color enableColor(FL_BLUE);
            fl_line(nX, nY, crossX, crossY);
        }

        {
            fl_wrapper::enable_color enableColor(FL_GREEN);
            fl_line(crossX - 5, crossY - 0, crossX + 5, crossY + 0);
            fl_line(crossX - 0, crossY - 5, crossX + 0, crossY + 5);
        }
    }
}

void PreviewWindow::resize()
{
    // resize the window
    // don't call this function in the ::draw()

    if(m_image){
        const int imgW = m_image->w();
        const int imgH = m_image->h();

        if(g_mainWindow->offsetDrawEnabled()){
            size(imgW * 2 + 40, imgH * 2 + 40);
        }
        else{
            const size_t winH = std::max<int>((std::min<int>((to_d(imgH * 1.5)), to_d(imgH + 40))), 200);
            const size_t winW = std::max<int>((std::min<int>((to_d(imgW * 1.5)), to_d(imgW + 40))), 200);
            size(winW, winH);
        }
    }
    else{
        size(100, 100);
    }
}

bool PreviewWindow::loadImage()
{
    if(!g_wilPackage->setIndex(g_mainWindow->selectedImageIndex())){
        return false;
    }

    const auto nW = g_wilPackage->currImageInfo()->width;
    const auto nH = g_wilPackage->currImageInfo()->height;

    m_imageOffX = g_wilPackage->currImageInfo()->px;
    m_imageOffY = g_wilPackage->currImageInfo()->py;

    m_imageBuf.clear();
    m_imageBuf.resize(nW * nH, 0X00000000);

    const auto layer = g_wilPackage->decode(false, g_mainWindow->removeShadowMosaicEnabled(), g_mainWindow->autoAlphaEnabled());

    if(layer[0] && g_mainWindow->layerIndexEnabled(0)){
        imgf::blendImageBuffer(m_imageBuf.data(), nW, nH, layer[0], nW, nH, 0, 0);
    }

    if(layer[1] && g_mainWindow->layerIndexEnabled(1)){
        imgf::blendImageBuffer(m_imageBuf.data(), nW, nH, layer[1], nW, nH, 0, 0);
    }

    if(layer[2] && g_mainWindow->layerIndexEnabled(2)){
        imgf::blendImageBuffer(m_imageBuf.data(), nW, nH, layer[2], nW, nH, 0, 0);
    }

    // Fl_RGB_Image won't copy the RGBA buffer
    // caller need to maintain m_buf when m_image is still used

    m_image.reset(Fl_RGB_Image((uchar *)(m_imageBuf.data()), nW, nH, 4).copy());
    m_imageIndex.emplace(g_mainWindow->selectedImageIndex());

    resize();
    copy_label((std::string("Index_") + std::to_string(m_imageIndex.value())).c_str());
    return true;
}
