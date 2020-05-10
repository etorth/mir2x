/*
 * =====================================================================================
 *
 *       Filename: serverluamodule.cpp
 *        Created: 12/19/2017 20:16:03
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

#include "monoserver.hpp"
#include "serverluamodule.hpp"

ServerLuaModule::ServerLuaModule()
    : LuaModule()
{
    m_luaState.set_function("getTime", []() -> int
    {
        extern MonoServer *g_monoServer;
        return (int)(g_monoServer->getCurrTick());
    });

    m_luaState.script(R"#(math.randomseed(getTime()))#");
}

void ServerLuaModule::addLog(int nLogType, const char *szLogInfo)
{
    if(!szLogInfo){
        szLogInfo = "";
    }

    // any time if you call addLog() in LUA
    // then this will get printed in the server GUI console

    extern MonoServer *g_monoServer;
    switch(nLogType){
        case 0  : g_monoServer->addLog(LOGTYPE_INFO   , "%s", szLogInfo); return;
        case 1  : g_monoServer->addLog(LOGTYPE_WARNING, "%s", szLogInfo); return;
        case 2  : g_monoServer->addLog(LOGTYPE_FATAL  , "%s", szLogInfo); return;
        default : g_monoServer->addLog(LOGTYPE_DEBUG  , "%s", szLogInfo); return;
    }
}
