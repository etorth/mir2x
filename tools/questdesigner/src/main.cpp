#include <ctime>
#include <string>
#include <cstdint>
#include <FL/Fl.H>
#include "qdmainwindow.hpp"

QD_MainWindow *g_mainWindow = nullptr;

int main()
{
    std::srand((unsigned int)(std::time(nullptr)));
    Fl::set_font(FL_HELVETICA, "文泉驿等宽正黑");

    g_mainWindow = new QD_MainWindow();
    g_mainWindow->showAll();

    return Fl::run();
}
