/*
 * =====================================================================================
 *
 *       Filename: clientluamodule.cpp
 *        Created: 06/25/2017 18:58:33
 *  Last Modified: 12/19/2017 22:41:24
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

#include "log.hpp"
#include "processrun.hpp"
#include "clientluamodule.hpp"

ClientLuaModule::ClientLuaModule(ProcessRun *pRun, int nOutPort)
    : LuaModule()
{
    if(true
            && pRun
            && nOutPort >= 0){

        // import all predefined lua functions / variables
        // don't need to keep ProcessRun and OutPort internally
        // where-ever it's in needed the lambda should capture it directly

        pRun->RegisterLuaExport(this, nOutPort);
    }
}

void ClientLuaModule::addLog(int nLogType, const char *szLogInfo)
{
    if(!szLogInfo){
        szLogInfo = "";
    }

    // any time if you call addLog() in LUA
    // then this will get printed in the server GUI console

    extern Log *g_Log;
    switch(nLogType){
        case 0  : g_Log->AddLog(LOGTYPE_INFO   , "%s", szLogInfo); return;
        case 1  : g_Log->AddLog(LOGTYPE_WARNING, "%s", szLogInfo); return;
        case 2  : g_Log->AddLog(LOGTYPE_FATAL  , "%s", szLogInfo); return;
        default : g_Log->AddLog(LOGTYPE_DEBUG  , "%s", szLogInfo); return;
    }
}
