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
#include "sysconst.hpp"
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
                w() / 2 + m_imageOffX - SYS_MAPGRIDXP / 2,
                h() / 2 + m_imageOffY - SYS_MAPGRIDYP / 2,
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
        const auto offX = nX - m_imageOffX;
        const auto offY = nY - m_imageOffY;
        const auto crossX = offX + SYS_MAPGRIDXP / 2;
        const auto crossY = offY + SYS_MAPGRIDYP / 2;
        {
            fl_wrapper::enable_color enableColor(FL_BLUE);
            fl_line(nX, nY, offX, offY, crossX, crossY);
        }

        {
            fl_wrapper::enable_color enableColor(FL_GREEN);
            fl_line(crossX - 5, crossY - 5, crossX + 5, crossY + 5);
            fl_line(crossX - 5, crossY + 5, crossX + 5, crossY - 5);
        }
    }
}

void PreviewWindow::autoResize()
{
    // resize the window
    // don't call this function in the ::draw()

    if(m_image){
        const int imgW = m_image->w();
        const int imgH = m_image->h();

        if(g_mainWindow->offsetDrawEnabled()){
            // the fixed cross point will be at the middle of the preview window
            // all other size/point get calculated by this fixed point
            //
            //     crossX = winW / 2
            //     crossY = winH / 2
            //
            //     crossX = drawStartX - m_imageOffX + SYS_MAPGRIDXP / 2
            //     crossY = drawStartY - m_imageOffY + SYS_MAPGRIDYP / 2
            //
            // =>
            //
            //     drawStartX = winW / 2 + m_imageOffX - SYS_MAPGRIDXP / 2
            //     drawStartY = winH / 2 + m_imageOffY - SYS_MAPGRIDYP / 2
            //
            // we need to make sure the image starts inside the window
            // also gives some margin at resize
            //
            // =>
            //
            //     winW / 2 + m_imageOffX - SYS_MAPGRIDXP / 2 >= margin
            //     winH / 2 + m_imageOffY - SYS_MAPGRIDYP / 2 >= margin
            //
            // =>
            //
            //     winW >= 2 * margin + SYS_MAPGRIDXP - m_imageOffX * 2     ----- (a)
            //     winH >= 2 * margin + SYS_MAPGRIDYP - m_imageOffY * 2
            //
            // we also need to ensure the image ends inside the window
            // also gives the same margin
            //
            // =>
            //
            //     drawStartX + imgW + margin <= winW
            //     drawStartY + imgH + margin <= winH
            //
            // =>
            //
            //     winW / 2 + m_imageOffX - SYS_MAPGRIDXP / 2 + imgW + margin <= winW
            //     winH / 2 + m_imageOffY - SYS_MAPGRIDYP / 2 + imgH + margin <= winH
            //
            // =>
            //
            //     winW >= m_imageOffX * 2 - SYS_MAPGRIDXP + imgW * 2 + margin * 2      ----- (b)
            //     winH >= m_imageOffY * 2 - SYS_MAPGRIDYP + imgH * 2 + margin * 2
            //
            // use (a) and (b) =>
            //
            //     winW >= std::max<int>(2 * margin + SYS_MAPGRIDXP - m_imageOffX * 2, m_imageOffX * 2 - SYS_MAPGRIDXP + imgW * 2 + margin * 2)
            //     winH >= std::max<int>(2 * margin + SYS_MAPGRIDYP - m_imageOffY * 2, m_imageOffY * 2 - SYS_MAPGRIDYP + imgH * 2 + margin * 2)
            //
            // simplify
            //
            //     winW >= margin * 2 + std::max<int>(SYS_MAPGRIDXP - m_imageOffX * 2, m_imageOffX * 2 - SYS_MAPGRIDXP + imgW * 2)
            //     winH >= margin * 2 + std::max<int>(SYS_MAPGRIDYP - m_imageOffY * 2, m_imageOffY * 2 - SYS_MAPGRIDYP + imgH * 2)

            // when we check if we need to resize
            // we assume margin can be zero, but when we decide to resize, we give non-zero margin, this prevent to resize at each frame

            const int minWinW = std::max<int>(SYS_MAPGRIDXP - m_imageOffX * 2, m_imageOffX * 2 - SYS_MAPGRIDXP + m_image->w() * 2);
            const int minWinH = std::max<int>(SYS_MAPGRIDYP - m_imageOffY * 2, m_imageOffY * 2 - SYS_MAPGRIDYP + m_image->h() * 2);

            if(w() < minWinW || h() < minWinH){
                const int margin = 100;
                size(margin * 2 + minWinW, margin * 2 + minWinH);
            }
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

    autoResize();
    copy_label((std::string("Index_") + std::to_string(m_imageIndex.value())).c_str());
    return true;
}
