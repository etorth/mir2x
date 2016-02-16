/*
 * =====================================================================================
 *
 *       Filename: previewwindow.cpp
 *        Created: 7/22/2015 3:16:57 AM
 *  Last Modified: 02/15/2016 17:21:33
 *
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
#include "previewwindow.hpp"
#include "wilimagepackage.hpp"
#include "mainwindow.hpp"

PreviewWindow::PreviewWindow(int W, int H)
	: Fl_Double_Window(0, 0, W, H)
    , m_Inited(false)
    , m_ImageIndex(0)
    , m_Data(nullptr)
    , m_DataLen(0)
    , m_DataMaxLen(0)
    , m_Image(nullptr)
{
}

PreviewWindow::~PreviewWindow()
{
    delete m_Data;
    delete m_Image;
}

void PreviewWindow::draw()
{
	Fl_Double_Window::draw();

    extern WilImagePackage  g_WilPackage;
    extern MainWindow      *g_MainWindow;

    if(!m_Inited ||
            (m_Inited && m_ImageIndex != g_MainWindow->SelectedImageIndex())){
        LoadImage();
    }

    if(m_Inited && m_ImageIndex == g_MainWindow->SelectedImageIndex() && m_Image){

        if(PlatformWindows() && !PlatformLinux()){
            // wtf
            make_current();
        }

        m_Image->draw(
                (w() - m_Image->w()) / 2,
                (h() - m_Image->h()) / 2,
                m_Image->w(), m_Image->h());
    }
}

void PreviewWindow::ExtendBuf(size_t nSize)
{
    if(m_DataMaxLen < nSize){
        delete m_Data;
    }
    m_Data       = new uint32_t[nSize];
    m_DataLen    = nSize;
    m_DataMaxLen = nSize;
}

void PreviewWindow::LoadImage()
{
    delete m_Image; m_Image = nullptr;

    extern WilImagePackage  g_WilPackage;
    extern MainWindow      *g_MainWindow;

    if(g_WilPackage.SetIndex(g_MainWindow->SelectedImageIndex())
            && g_WilPackage.CurrentImageValid()){
        auto W = g_WilPackage.CurrentImageInfo().shWidth;
        auto H = g_WilPackage.CurrentImageInfo().shHeight;

        ExtendBuf((size_t)(W * H));

        g_WilPackage.Decode(m_Data, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);
        m_Image = new Fl_RGB_Image((uchar *)m_Data, W, H, 4);
    }

    m_Inited = false;
    if(m_Image){
        m_Inited = true;
        m_ImageIndex = g_MainWindow->SelectedImageIndex();
    }
}
