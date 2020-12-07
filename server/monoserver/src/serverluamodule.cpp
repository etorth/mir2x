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

extern MonoServer *g_monoServer;

ServerLuaModule::ServerLuaModule()
    : LuaModule()
{
    m_luaState.set_function("getTime", []() -> int
    {
        return (int)(g_monoServer->getCurrTick());
    });

    m_luaState.script(R"#(math.randomseed(getTime()))#");
}

void ServerLuaModule::addLog(int nLogType, const char8_t *logInfo)
{
    if(!logInfo){
        logInfo = u8"";
    }

    // any time if you call addLog() in LUA
    // then this will get printed in the server GUI console

    switch(nLogType){
        case 0  : g_monoServer->addLog(LOGTYPE_INFO   , "%s", logInfo); return;
        case 1  : g_monoServer->addLog(LOGTYPE_WARNING, "%s", logInfo); return;
        case 2  : g_monoServer->addLog(LOGTYPE_FATAL  , "%s", logInfo); return;
        default : g_monoServer->addLog(LOGTYPE_DEBUG  , "%s", logInfo); return;
    }
}
