/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 08/31/2015 08:52:57 PM
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
#include <iostream>

#include "mainwindow.hpp"
#include "previewwindow.hpp"
#include "wilimagepackage.hpp"
#include "progressbarwindow.hpp"

std::string          g_FileFullName;
WilImagePackage      g_WilPackage;
MainWindow          *g_MainWindow;
PreviewWindow       *g_PreviewWindow;
ProgressBarWindow   *g_ProgressBarWindow;

int main()
{
    g_FileFullName      = "";
    g_MainWindow        = nullptr;
    g_PreviewWindow     = nullptr;
    g_ProgressBarWindow = nullptr;

    Fl::visual(FL_RGB | FL_ALPHA);

    g_ProgressBarWindow = new ProgressBarWindow();
    g_MainWindow        = new MainWindow();
    g_PreviewWindow     = new PreviewWindow();

    g_PreviewWindow->hide();
    g_MainWindow->ShowAll();

    return Fl::run();
}
