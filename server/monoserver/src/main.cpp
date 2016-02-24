#include <ctime>
#include "asio.hpp"
#include "mainwindow.hpp"
#include "addmonsterwindow.hpp"
#include "serverconfigurewindow.hpp"
#include "networkconfigurewindow.hpp"

MonoServer              *g_MonoServer;
MainWindow              *g_MainWindow;
ServerConfigureWindow   *g_ServerConfigureWindow;
NetworkConfigureWindow  *g_NetworkConfigureWindow;

int main()
{
    std::srand(std::time(nullptr));

    g_MainWindow             = new MainWindow();
    g_MonoServer             = new MonoServer();
    g_ServerConfigureWindow  = new ServerConfigureWindow();
    g_NetworkConfigureWindow = new NetworkConfigureWindow();

    g_MainWindow->ShowAll();

    return Fl::run();
}
