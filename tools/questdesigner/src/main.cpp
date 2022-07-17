#include <ctime>
#include <string>
#include <cstdint>
#include <FL/Fl.H>
#include "mainwindow.hpp"

MainWindow *g_mainWindow = nullptr;

int main()
{
    std::srand((unsigned int)(std::time(nullptr)));
    Fl::set_font(FL_HELVETICA, "文泉驿等宽正黑");

    g_mainWindow = new MainWindow();
    g_mainWindow->showAll();

    return Fl::run();
}
