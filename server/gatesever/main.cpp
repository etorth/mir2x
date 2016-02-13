#include "asio.hpp"
#include "mainwindow.hpp"
#include "configurewindow.hpp"
#include <ctime>

GateServer      *g_GateServer;
MainWindow      *g_MainWindow;
ConfigureWindow *g_ConfigureWindow;

int main()
{
	std::srand(std::time(nullptr));
    g_MainWindow      = new MainWindow();
    g_ConfigureWindow = new ConfigureWindow();
	g_MainWindow->ShowAll();

    return Fl::run();

}
