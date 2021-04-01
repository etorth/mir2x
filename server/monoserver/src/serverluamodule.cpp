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

#include "dbcomid.hpp"
#include "mapbindb.hpp"
#include "monoserver.hpp"
#include "serverluamodule.hpp"
#include "serverconfigurewindow.hpp"

extern MapBinDB *g_mapBinDB;
extern MonoServer *g_monoServer;
extern ServerConfigureWindow *g_serverConfigureWindow;

ServerLuaModule::ServerLuaModule()
    : LuaModule()
{
    m_luaState.script(str_printf("package.path = package.path .. ';%s/?.lua'", []() -> std::string
    {
        if(const auto cfgScriptPath = g_serverConfigureWindow->getScriptPath(); cfgScriptPath.empty()){
            return "script";
        }
        else{
            return cfgScriptPath;
        }
    }().c_str()));

    m_luaState.set_function("randMapGLoc", [](std::string mapName)
    {
        const auto fnGetRandGLoc = [](const auto dataCPtr) -> std::array<int, 2>
        {
            while(true){
                const int x = std::rand() % dataCPtr->W();
                const int y = std::rand() % dataCPtr->H();

                if(true
                        && dataCPtr->Valid()
                        && dataCPtr->ValidC(x, y)
                        && dataCPtr->Cell(x, y).CanThrough()){
                    return {x, y};
                }
            }
            throw bad_reach();
        };

        if(const auto mapID = DBCOM_MAPID(to_u8cstr(mapName))){
            if(const auto dataCPtr = g_mapBinDB->Retrieve(mapID)){
                return sol::as_returns(fnGetRandGLoc(dataCPtr));
            }
            else{
                throw fflerror("map %s has no valid mir2x data", to_cstr(mapName));
            }
        }
        else{
            throw fflerror("invalid map name: %s", to_cstr(mapName));
        }
    });
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
