/*
 * =====================================================================================
 *
 *       Filename: globals.hpp
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

#pragma once
#include "log.hpp"
#include "dbpod.hpp"
#include "threadpn.hpp"
#include "actorpool.hpp"
#include "netdriver.hpp"

#include "memorypn.hpp"
#include "mapbindbn.hpp"
#include "monoserver.hpp"
#include "serverargparser.hpp"

#include "mainwindow.hpp"
#include "scriptwindow.hpp"
#include "serverconfigurewindow.hpp"
#include "databaseconfigurewindow.hpp"

extern Log *g_Log;
extern DBPodN *g_DBPodN;
extern ThreadPN *g_ThreadPN;
extern ActorPool *g_ActorPool;

extern NetDriver *g_NetDriver;
extern ServerArgParser *g_ServerArgParser;

extern MemoryPN *g_MemoryPN;
extern MapBinDBN *g_MapBinDBN;
extern MonoServer *g_MonoServer;

extern MainWindow *g_MainWindow;
extern ScriptWindow *g_ScriptWindow;
extern ServerConfigureWindow *g_ServerConfigureWindow;
extern DatabaseConfigureWindow *g_DatabaseConfigureWindow;
