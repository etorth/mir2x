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
#include <asio.hpp>
#include <ctime>

#include "log.hpp"
#include "dbpod.hpp"
#include "memorypn.hpp"
#include "threadpn.hpp"
#include "mapbindbn.hpp"
#include "actorpool.hpp"
#include "netdriver.hpp"
#include "argparser.hpp"
#include "mainwindow.hpp"
#include "scriptwindow.hpp"
#include "serverargparser.hpp"
#include "serverconfigurewindow.hpp"
#include "databaseconfigurewindow.hpp"

#include <iostream>
#include "coro.hpp"

ServerArgParser          *g_ServerArgParser;
Log                      *g_Log;
MemoryPN                 *g_MemoryPN;
ActorPool                *g_ActorPool;
ThreadPN                 *g_ThreadPN;
NetDriver                *g_NetDriver;
DBPodN                   *g_DBPodN;

MapBinDBN                *g_MapBinDBN;
ScriptWindow             *g_ScriptWindow;
MainWindow               *g_MainWindow;
MonoServer               *g_MonoServer;
ServerConfigureWindow    *g_ServerConfigureWindow;
DatabaseConfigureWindow  *g_DatabaseConfigureWindow;
ActorMonitorWindow       *g_ActorMonitorWindow;
ActorThreadMonitorWindow *g_ActorThreadMonitorWindow;

#include "coro.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    std::srand((unsigned int)std::time(nullptr));
    try{
        arg_parser stCmdParser(argc, argv);

        // start FLTK multithreading support
        Fl::lock();

        g_ServerArgParser          = new ServerArgParser(stCmdParser);
        g_Log                      = new Log("mir2x-monoserver-v0.1");
        g_ScriptWindow             = new ScriptWindow();
        g_MainWindow               = new MainWindow();
        g_MonoServer               = new MonoServer();
        g_MemoryPN                 = new MemoryPN();
        g_MapBinDBN                = new MapBinDBN();
        g_ServerConfigureWindow    = new ServerConfigureWindow();
        g_DatabaseConfigureWindow  = new DatabaseConfigureWindow();
        g_ActorPool                = new ActorPool(g_ServerArgParser->ActorPoolThread);
        g_ThreadPN                 = new ThreadPN(4);
        g_DBPodN                   = new DBPodN();
        g_NetDriver                = new NetDriver();
        g_ActorMonitorWindow       = new ActorMonitorWindow();
        g_ActorThreadMonitorWindow = new ActorThreadMonitorWindow();

        g_MainWindow->ShowAll();

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
                            g_MonoServer->DetectException();
                        }catch(const std::exception &rstException){
                            g_MonoServer->LogException(rstException);
                            g_MonoServer->Restart();
                        }
                        break;
                    }
                case 1:
                default:
                    {
                        // pase the gui requests in the queue
                        // designed to send Fl::awake(1) to notify gui
                        g_MonoServer->ParseNotifyGUIQ();
                        break;
                    }
            }
        }
    }catch(const std::exception &e){
        // use raw log directly
        // no gui available because we are out of gui event loop
        g_Log->AddLog(LOGTYPE_WARNING, "Exception in main thread: %s", e.what());
    }catch(...){
        g_Log->AddLog(LOGTYPE_WARNING, "Unknown exception caught in main thread");
    }
    return 0;
}
