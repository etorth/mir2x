/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 05/09/2017 19:38:08
 *
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
#include "netpod.hpp"
#include "taskhub.hpp"
#include "memorypn.hpp"
#include "threadpn.hpp"
#include "metronome.hpp"
#include "mainwindow.hpp"
#include "eventtaskhub.hpp"
#include "addmonsterwindow.hpp"
#include "serverconfigurewindow.hpp"
#include "databaseconfigurewindow.hpp"

Log                      *g_Log;
TaskHub                  *g_TaskHub;
MemoryPN                 *g_MemoryPN;
EventTaskHub             *g_EventTaskHub;
Theron::EndPoint         *g_EndPoint;
Theron::Framework        *g_Framework;
ThreadPN                 *g_ThreadPN;
NetPodN                  *g_NetPodN;
DBPodN                   *g_DBPodN;

MainWindow               *g_MainWindow;
MonoServer               *g_MonoServer;
AddMonsterWindow         *g_AddMonsterWindow;
ServerConfigureWindow    *g_ServerConfigureWindow;
DatabaseConfigureWindow  *g_DatabaseConfigureWindow;


int main()
{
    std::srand(std::time(nullptr));

    // start FLTK multithreading support
    Fl::lock();

    g_Log                     = new Log("mir2x-monoserver-v0.1");
    g_TaskHub                 = new TaskHub();
    g_MainWindow              = new MainWindow();
    g_MonoServer              = new MonoServer();
    g_AddMonsterWindow        = new AddMonsterWindow();
    g_MemoryPN                = new MemoryPN();
    g_ServerConfigureWindow   = new ServerConfigureWindow();
    g_DatabaseConfigureWindow = new DatabaseConfigureWindow();
    g_EventTaskHub            = new EventTaskHub();
    g_EndPoint                = new Theron::EndPoint("monoserver", "tcp://127.0.0.1:5556");
    g_Framework               = new Theron::Framework(*g_EndPoint);
    g_ThreadPN                = new ThreadPN(4);
    g_DBPodN                  = new DBPodN();
    g_NetPodN                 = new NetPodN();

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
            case 1:
                {
                    // request to stop
                    // FLTK will abort() if got 1
                    fl_alert("%s", "system request for restart");
                    exit(0);
                    break;
                }
            case 2:
                {
                    // to show log info
                    // can't do it in child thread since it blocks, why?
                    g_MonoServer->FlushBrowser();
                    break;
                }
            default:
                {
                    break;
                }
        }
    }

    return 0;
}
