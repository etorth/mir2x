#include <iostream>
#include <string>
#include "mainwindow.hpp"
#include "wilfilewindow.hpp"
#include "wilimagepackage.hpp"
#include "previewwindow.hpp"

std::string          g_FileFullName;
WilImagePackage      g_WilPackage;
MainWindow          *g_MainWindow;
Fl_Double_Window    *g_DrawWindow;
PreviewWindow       *g_PreviewWindow;

int main()
{
    Fl::visual(FL_RGB | FL_ALPHA);

    g_MainWindow = new MainWindow();
    g_MainWindow->ShowAll();
    Fl::run();
    return 0;
}
