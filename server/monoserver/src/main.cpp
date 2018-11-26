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

#include "argh.h"
#include "log.hpp"
#include "dbpod.hpp"
#include "memorypn.hpp"
#include "threadpn.hpp"
#include "mapbindbn.hpp"
#include "actorpool.hpp"
#include "netdriver.hpp"
#include "mainwindow.hpp"
#include "scriptwindow.hpp"
#include "serverargparser.hpp"
#include "serverconfigurewindow.hpp"
#include "databaseconfigurewindow.hpp"

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


int main(int argc, char *argv[])
{
    std::srand((unsigned int)std::time(nullptr));
    argh::parser stCmdParser(argc, argv);

    // start FLTK multithreading support
    Fl::lock();

    g_ServerArgParser         = new ServerArgParser(stCmdParser);
    g_Log                     = new Log("mir2x-monoserver-v0.1");
    g_ScriptWindow            = new ScriptWindow();
    g_MainWindow              = new MainWindow();
    g_MonoServer              = new MonoServer();
    g_MemoryPN                = new MemoryPN();
    g_MapBinDBN               = new MapBinDBN();
    g_ServerConfigureWindow   = new ServerConfigureWindow();
    g_DatabaseConfigureWindow = new DatabaseConfigureWindow();
    g_ActorPool               = new ActorPool(g_ServerArgParser->ActorPoolThread);
    g_ThreadPN                = new ThreadPN(4);
    g_DBPodN                  = new DBPodN();
    g_NetDriver               = new NetDriver();

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
            default:
                {
                    // designed to send Fl::awake(1) to notify gui
                    // to pase the requests in the cached queue
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->ParseNotifyGUIQ();
                    break;
                }
        }
    }
    return 0;
}
