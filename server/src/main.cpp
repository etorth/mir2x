#include <ctime>
#include <asio.hpp>
#include "log.hpp"
#include "argf.hpp"
#include "dbpod.hpp"
#include "mapbindb.hpp"
#include "actorpool.hpp"
#include "mainwindow.hpp"
#include "scriptwindow.hpp"
#include "profilerwindow.hpp"
#include "serverargparser.hpp"
#include "podmonitorwindow.hpp"
#include "serverpasswordwindow.hpp"
#include "serverconfigurewindow.hpp"

ServerArgParser          *g_serverArgParser;
Log                      *g_log;
ActorPool                *g_actorPool;
DBPod                    *g_dbPod;

MapBinDB                 *g_mapBinDB;
ScriptWindow             *g_scriptWindow;
ProfilerWindow           *g_profilerWindow;
MainWindow               *g_mainWindow;
MonoServer               *g_monoServer;
ServerPasswordWindow     *g_serverPasswordWindow;
ServerConfigureWindow    *g_serverConfigureWindow;
PodMonitorWindow         *g_podMonitorWindow;
ActorMonitorWindow       *g_actorMonitorWindow;

int main(int argc, char *argv[])
{
    std::srand((unsigned int)std::time(nullptr));
    try{
        argf::parser cmdParser(argc, argv);
        g_serverArgParser = new ServerArgParser(cmdParser);

        if(g_serverArgParser->disableProfiler){
            logDisableProfiler();
        }

        std::atexit(+[]()
        {
            logProfiling([](const std::string &s)
            {
                std::printf("%s", s.c_str());
            });
        });

        if(!g_serverArgParser->slave){
            // start FLTK multithreading support
            // for more details: https://www.fltk.org/doc-1.4/advanced.html
            Fl::lock();
        }

        g_log        = new Log("mir2x-monoserver-v0.1");
        g_monoServer = new MonoServer();
        g_mapBinDB   = new MapBinDB();
        g_actorPool  = new ActorPool(g_serverArgParser->actorPoolThread, g_serverArgParser->logicalFPS);
        g_dbPod      = new DBPod();

        if(!g_serverArgParser->slave){
            g_scriptWindow          = new ScriptWindow();
            g_profilerWindow        = new ProfilerWindow();
            g_mainWindow            = new MainWindow();
            g_serverPasswordWindow  = new ServerPasswordWindow();
            g_serverConfigureWindow = new ServerConfigureWindow();
            g_podMonitorWindow      = new PodMonitorWindow();
            g_actorMonitorWindow    = new ActorMonitorWindow();

            if(g_serverArgParser->textFont >= 0){
                g_mainWindow->setGUIFont(g_serverArgParser->textFont);
            }

            g_mainWindow->showAll();
            if(g_serverArgParser->autoLaunch){
                g_monoServer->launch();
            }
        }

        g_monoServer->mainLoop();
    }
    catch(const std::exception &e){
        // use raw log directly
        // no gui available because we are out of gui event loop
        g_log->addLog(LOGTYPE_WARNING, "Exception in main thread: %s", e.what());
    }
    catch(...){
        g_log->addLog(LOGTYPE_WARNING, "Unknown exception caught in main thread");
    }
    return 0;
}
