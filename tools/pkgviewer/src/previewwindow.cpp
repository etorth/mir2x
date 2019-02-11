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

#include "platforms.hpp"
#include "mainwindow.hpp"
#include "previewwindow.hpp"
#include "wilimagepackage.hpp"

// to remove wired black pixels for magic wil images
// implement here instead of put in WilImagePackage::Decode()
static void CalcAutoAlpha(uint32_t *pData, size_t nDataLen)
{
    if(!(pData && nDataLen)){
        return;
    }

    for(size_t nIndex = 0; nIndex < nDataLen; ++nIndex){
        uint8_t a = ((pData[nIndex] & 0XFF000000) >> 24);

        if(a == 0){
            continue;
        }

        uint8_t r = ((pData[nIndex] & 0X00FF0000) >> 16);
        uint8_t g = ((pData[nIndex] & 0X0000FF00) >>  8);
        uint8_t b = ((pData[nIndex] & 0X000000FF) >>  0);

        uint32_t lum = std::lround(0.30 * r + 0.59 * g + 0.11 * b);

        pData[nIndex] &= 0X00FFFFFF;
        pData[nIndex] |= (lum << 24);
    }
}

PreviewWindow::PreviewWindow(int W, int H)
	: Fl_Double_Window(0, 0, W, H)
    , m_Inited(false)
    , m_ImageIndex(0)
    , m_Image()
    , m_Buf()
{}

void PreviewWindow::draw()
{
	Fl_Double_Window::draw();

    extern WilImagePackage  g_WilPackage;
    extern MainWindow      *g_MainWindow;

    if(!m_Inited || (m_Inited && m_ImageIndex != g_MainWindow->SelectedImageIndex())){
        LoadImage();
    }

    if(m_Inited && m_ImageIndex == g_MainWindow->SelectedImageIndex() && m_Image){

        // for howto use make_current(), see my understanding as comments in:
        //      tools/mapeditor/src/animationpreviewarea.cpp
        // put this here as unchanged since it works properly
        if(PlatformWindows() && !PlatformLinux()){
            // wtf
            make_current();
        }

        int nX = (w() - m_Image->w()) / 2;
        int nY = (h() - m_Image->h()) / 2;
        int nW = m_Image->w();
        int nH = m_Image->h();

        m_Image->draw(nX, nY, nW, nH);
        auto stColor = fl_color();
        fl_color(FL_RED);
        fl_rect(nX, nY, nW, nH);
        fl_color(stColor);
    }
}

void PreviewWindow::LoadImage()
{
    extern WilImagePackage  g_WilPackage;
    extern MainWindow      *g_MainWindow;

    if(true
            && g_WilPackage.SetIndex(g_MainWindow->SelectedImageIndex())
            && g_WilPackage.CurrentImageValid()){

        auto nW = g_WilPackage.CurrentImageInfo().shWidth;
        auto nH = g_WilPackage.CurrentImageInfo().shHeight;

        m_Buf.resize(0);
        m_Buf.resize(nW * nH);

        g_WilPackage.Decode(m_Buf.data(), 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);
        if(g_MainWindow->AutoAlpha()){
            CalcAutoAlpha(m_Buf.data(), m_Buf.size());
        }
        m_Image = std::make_unique<Fl_RGB_Image>((uchar *)(m_Buf.data()), nW, nH, 4);
    }

    m_Inited = false;
    if(m_Image){
        m_Inited = true;
        m_ImageIndex = g_MainWindow->SelectedImageIndex();
    }
}
