#include "asio.hpp"
#include "newmir2map.hpp"
#include "mainwindow.hpp"
#include "addmonsterwindow.hpp"
#include "serverconfigurewindow.hpp"
#include "networkconfigurewindow.hpp"

SceneServer             *g_SceneServer;
MainWindow              *g_MainWindow;
NetworkConfigureWindow  *g_NetworkConfigureWindow;
ServerConfigureWindow   *g_ServerConfigureWindow;
AddMonsterWindow        *g_AddMonsterWindow;
NewMir2Map               g_Map;

int main()
{
    std::srand(std::time(nullptr));

    g_MainWindow             = new MainWindow();
    g_AddMonsterWindow       = new AddMonsterWindow();
    g_NetworkConfigureWindow = new NetworkConfigureWindow();
    g_ServerConfigureWindow  = new ServerConfigureWindow();
	g_MainWindow->ShowAll();

    return Fl::run();
}
