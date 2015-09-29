/*
 * =====================================================================================
 *
 *       Filename: previewwindow.cpp
 *        Created: 7/22/2015 3:16:57 AM
 *  Last Modified: 08/01/2015 5:42:50 PM
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

#include "previewwindow.hpp"
#include "wilimagepackage.hpp"
#include "mainwindow.hpp"

PreviewWindow::PreviewWindow(int W, int H)
	: Fl_Double_Window(0, 0, W, H)
    , m_Data(nullptr)
    , m_DataLen(0)
{
}

void PreviewWindow::draw()
{
	Fl_Double_Window::draw();

    extern WilImagePackage  g_WilPackage;
    extern MainWindow      *g_MainWindow;

    g_WilPackage.SetIndex(g_MainWindow->SelectedImageIndex());
    if(g_WilPackage.CurrentImageValid()){
        auto W = g_WilPackage.CurrentImageInfo().shWidth;
        auto H = g_WilPackage.CurrentImageInfo().shHeight;
        if(m_DataLen < W * H){
            delete m_Data;
        }
        m_Data    = new uint32_t[W * H];
        m_DataLen = W * H;
        g_WilPackage.Decode(m_Data, 0XFFFFFFFF, 0XFFFFFFFF);

		make_current();
		fl_draw_image((uint8_t *)(m_Data), (w() - W) / 2, (h() - H) / 2, W, H, 4, W * 4);
    }
}

PreviewWindow::~PreviewWindow()
{
    delete m_Data;
}
