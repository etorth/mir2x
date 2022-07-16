#include <string>
#include <cstdint>
#include <FL/Fl.H>

#include "mainwindow.hpp"

MainWindow                      *g_mainWindow               = nullptr;

int main()
{
    g_mainWindow            = new MainWindow();

    g_mainWindow->showAll();
    return Fl::run();
}
