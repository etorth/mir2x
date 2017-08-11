/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 08/10/2017 20:09:19
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

#include <string>
#include <iostream>

#include "mainwindow.hpp"
#include "previewwindow.hpp"
#include "wilimagepackage.hpp"

std::string          g_FileFullName;
WilImagePackage      g_WilPackage;
MainWindow          *g_MainWindow;
PreviewWindow       *g_PreviewWindow;

int main()
{
    g_FileFullName  = "";
    g_MainWindow    = nullptr;
    g_PreviewWindow = nullptr;

    Fl::visual(FL_RGB | FL_ALPHA);

    g_MainWindow = new MainWindow();
    g_MainWindow->ShowAll();
    Fl::run();
    return 0;
}
