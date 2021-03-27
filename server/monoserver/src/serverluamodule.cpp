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
#include "serverconfigurewindow.hpp"

extern MonoServer *g_monoServer;
extern ServerConfigureWindow *g_serverConfigureWindow;
ServerLuaModule::ServerLuaModule()
    : LuaModule()
{
    m_luaState.set_function("getTime", []() -> int
    {
        return to_d(g_monoServer->getCurrTick());
    });

    m_luaState.script(R"#(math.randomseed(getTime()))#");
    m_luaState.script(str_printf("package.path = package.path .. ';%s/?.lua'", []() -> std::string
    {
        if(const auto cfgScriptPath = g_serverConfigureWindow->getScriptPath(); cfgScriptPath.empty()){
            return "script";
        }
        else{
            return cfgScriptPath;
        }
    }().c_str()));
}

void ServerLuaModule::addLogString(int nLogType, const char8_t *logInfo)
{
    // any time if you call addLog() or addLogString() in lua module
    // this will get printed in the server GUI console

    switch(nLogType){
        case 0  : g_monoServer->addLog(LOGTYPE_INFO   , "%s", to_cstr(logInfo)); return;
        case 1  : g_monoServer->addLog(LOGTYPE_WARNING, "%s", to_cstr(logInfo)); return;
        case 2  : g_monoServer->addLog(LOGTYPE_FATAL  , "%s", to_cstr(logInfo)); return;
        default : g_monoServer->addLog(LOGTYPE_DEBUG  , "%s", to_cstr(logInfo)); return;
    }
}
