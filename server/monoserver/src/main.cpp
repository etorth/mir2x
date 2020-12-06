/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */
#include <ctime>
#include <asio.hpp>
#include "log.hpp"
#include "dbpod.hpp"
#include "mapbindb.hpp"
#include "actorpool.hpp"
#include "netdriver.hpp"
#include "argparser.hpp"
#include "mainwindow.hpp"
#include "scriptwindow.hpp"
#include "profilerwindow.hpp"
#include "serverargparser.hpp"
#include "podmonitorwindow.hpp"
#include "serverconfigurewindow.hpp"
#include "databaseconfigurewindow.hpp"

ServerArgParser          *g_serverArgParser;
Log                      *g_log;
ActorPool                *g_actorPool;
NetDriver                *g_netDriver;
DBPod                    *g_DBPod;

MapBinDB                 *g_mapBinDB;
ScriptWindow             *g_scriptWindow;
ProfilerWindow           *g_profilerWindow;
MainWindow               *g_mainWindow;
MonoServer               *g_monoServer;
ServerConfigureWindow    *g_serverConfigureWindow;
DatabaseConfigureWindow  *g_databaseConfigureWindow;
PodMonitorWindow         *g_podMonitorWindow;
ActorMonitorWindow       *g_actorMonitorWindow;

int main(int argc, char *argv[])
{
    std::srand((unsigned int)std::time(nullptr));
    try{
        arg_parser cmdParser(argc, argv);
        g_serverArgParser = new ServerArgParser(cmdParser);

        if(g_serverArgParser->disableProfiler){
            logDisableProfiler();
        }

        // start FLTK multithreading support
        Fl::lock();

        g_log                      = new Log("mir2x-monoserver-v0.1");
        g_scriptWindow             = new ScriptWindow();
        g_profilerWindow           = new ProfilerWindow();
        g_mainWindow               = new MainWindow();
        g_monoServer               = new MonoServer();
        g_mapBinDB                 = new MapBinDB();
        g_serverConfigureWindow    = new ServerConfigureWindow();
        g_databaseConfigureWindow  = new DatabaseConfigureWindow();
        g_actorPool                = new ActorPool(g_serverArgParser->actorPoolThread, 10);
        g_DBPod                    = new DBPod();
        g_netDriver                = new NetDriver();
        g_podMonitorWindow         = new PodMonitorWindow();
        g_actorMonitorWindow       = new ActorMonitorWindow();

        std::atexit(+[]()
        {
            logProfiling([](const std::string &s)
            {
                std::printf("%s", s.c_str());
            });
        });
        g_mainWindow->showAll();

        while(Fl::wait() > 0){
            switch((uintptr_t)(Fl::thread_message())){
                case 0:
                    {
                        // FLTK will send 0 automatically
                        // to update the widgets and handle events
                        //
                        // if main loop or child thread need to flush
                        // call Fl::awake(0) to force Fl::wait() to terminate
                        break;
                    }
                case 2:
                    {
                        // propagate all exceptions to main thread
                        // then log it in main thread and request restart
                        //
                        // won't handle exception in threads
                        // all threads need to call Fl::awake(2) to propagate exception(s) caught
                        try{
                            g_monoServer->checkException();
                        }catch(const std::exception &except){
                            std::string firstExceptStr;
                            g_monoServer->logException(except, &firstExceptStr);
                            g_monoServer->restart(firstExceptStr);
                        }
                        break;
                    }
                case 1:
                default:
                    {
                        // pase the gui requests in the queue
                        // designed to send Fl::awake(1) to notify gui
                        g_monoServer->parseNotifyGUIQ();
                        break;
                    }
            }
        }
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
