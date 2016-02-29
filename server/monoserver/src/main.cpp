#include <ctime>
#include "asio.hpp"
#include "mainwindow.hpp"
#include "addmonsterwindow.hpp"
#include "serverconfigurewindow.hpp"
#include "databaseconfigurewindow.hpp"

MonoServer               *g_MonoServer;
MainWindow               *g_MainWindow;
ServerConfigureWindow    *g_ServerConfigureWindow;
DatabaseConfigureWindow  *g_DatabaseConfigureWindow;

int main()
{
    std::srand(std::time(nullptr));

    g_MainWindow              = new MainWindow();
    g_MonoServer              = new MonoServer();
    g_ServerConfigureWindow   = new ServerConfigureWindow();
    g_DatabaseConfigureWindow = new DatabaseConfigureWindow();

    g_MainWindow->ShowAll();

    return Fl::run();
}
