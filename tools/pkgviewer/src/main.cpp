#include <string>
#include <iostream>

#include "mainwindow.hpp"
#include "previewwindow.hpp"
#include "wilimagepackage.hpp"
#include "progressbarwindow.hpp"

std::string          g_fileFullName;
WilImagePackage     *g_wilPackage;
MainWindow          *g_mainWindow;
PreviewWindow       *g_previewWindow;
ProgressBarWindow   *g_progressBarWindow;

int main()
{
    g_fileFullName      = "";
    g_wilPackage        = nullptr;
    g_mainWindow        = nullptr;
    g_previewWindow     = nullptr;
    g_progressBarWindow = nullptr;

    Fl::visual(FL_RGB | FL_ALPHA);

    g_progressBarWindow = new ProgressBarWindow();
    g_previewWindow     = new PreviewWindow();
    g_mainWindow        = new MainWindow();

    g_previewWindow->hide();
    g_progressBarWindow->hideAll();

    g_mainWindow->showAll();
    return Fl::run();
}
