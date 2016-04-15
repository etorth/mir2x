#include <ctime>
#include <asio.hpp>

#include "log.hpp"
#include "taskhub.hpp"
#include "mainwindow.hpp"
#include "eventtaskhub.hpp"
#include "monitorwindow.hpp"
#include "addmonsterwindow.hpp"
#include "serverconfigurewindow.hpp"
#include "databaseconfigurewindow.hpp"


Log                      *g_Log;
TaskHub                  *g_TaskHub;
EventTaskHub             *g_EventTaskHub;

MainWindow               *g_MainWindow;
MonoServer               *g_MonoServer;
MonitorWindow            *g_MonitorWindow;
AddMonsterWindow         *g_AddMonsterWindow;
ServerConfigureWindow    *g_ServerConfigureWindow;
DatabaseConfigureWindow  *g_DatabaseConfigureWindow;

int main()
{
    std::srand(std::time(nullptr));

    g_Log                     = new Log("mir2x-monoserver-v0.1");
    g_TaskHub                 = new TaskHub();
    g_MainWindow              = new MainWindow();
    g_MonoServer              = new MonoServer();
    g_MonitorWindow           = new MonitorWindow();
    g_ServerConfigureWindow   = new ServerConfigureWindow();
    g_DatabaseConfigureWindow = new DatabaseConfigureWindow();
    g_EventTaskHub            = new EventTaskHub();

    g_MainWindow->ShowAll();

    return Fl::run();
}
