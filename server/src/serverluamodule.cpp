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

#include "dbpod.hpp"
#include "dbcomid.hpp"
#include "mapbindb.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"
#include "serverluamodule.hpp"
#include "serverconfigurewindow.hpp"

extern DBPod *g_dbPod;
extern MapBinDB *g_mapBinDB;
extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;
extern ServerConfigureWindow *g_serverConfigureWindow;

ServerLuaModule::ServerLuaModule()
    : LuaModule()
{
    m_luaState.script(str_printf("package.path = package.path .. ';%s/?.lua'", []() -> std::string
    {
        if(const auto cfgScriptPath = g_serverConfigureWindow->getConfig().scriptPath; cfgScriptPath.empty()){
            return "script";
        }
        else{
            return cfgScriptPath;
        }
    }().c_str()));

    m_luaState.set_function("isUIDAlive", [](std::string uidString)
    {
        return g_actorPool->checkUIDValid(uidf::toUIDEx(uidString));
    });

    m_luaState.set_function("randMapGLoc", [](std::string mapName)
    {
        const auto fnGetRandGLoc = [](const auto dataCPtr) -> std::array<int, 2>
        {
            while(true){
                const int x = std::rand() % dataCPtr->w();
                const int y = std::rand() % dataCPtr->h();

                if(dataCPtr->validC(x, y) && dataCPtr->cell(x, y).land.canThrough()){
                    return {x, y};
                }
            }
            throw fflreach();
        };

        if(const auto mapID = DBCOM_MAPID(to_u8cstr(mapName))){
            if(const auto dataCPtr = g_mapBinDB->retrieve(mapID)){
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

    m_luaState.set_function("hasDatabase", [](std::string dbName) -> bool
    {
        fflassert(!dbName.empty());
        return g_dbPod->createQuery(u8R"###(select name from sqlite_master where type='table' and name='%s')###", to_cstr(dbName)).executeStep();
    });

    m_luaState.set_function("dbExecString", [](std::string cmd)
    {
        fflassert(!cmd.empty());
        g_dbPod->exec(to_cstr(cmd));
    });

    m_luaState.set_function("dbQueryString", [](std::string query, sol::this_state s)
    {
        fflassert(!query.empty());
        auto queryStatement = g_dbPod->createQuery(to_cstr(query));

        sol::state_view sv(s);
        std::vector<std::map<std::string, sol::object>> queryResult;

        queryResult.reserve(8);
        while(queryStatement.executeStep()){
            std::map<std::string, sol::object> rowResult;
            for(int i = 0; i < queryStatement.getColumnCount(); ++i){
                switch(const auto column = queryStatement.getColumn(i); column.getType()){
                    case SQLITE_INTEGER:
                        {
                            rowResult[column.getName()] = sol::object(sv, sol::in_place_type<int>, column.getInt());
                            break;
                        }
                    case SQLITE_FLOAT:
                        {
                            rowResult[column.getName()] = sol::object(sv, sol::in_place_type<double>, column.getDouble());
                            break;
                        }
                    case SQLITE_TEXT:
                        {
                            rowResult[column.getName()] = sol::object(sv, sol::in_place_type<std::string>, column.getText());
                            break;
                        }
                    default:
                        {
                            throw fflerror("column type not supported: %d", column.getType());
                        }
                }
            }

            if(rowResult.size() != to_uz(queryStatement.getColumnCount())){
                throw fflerror("failed to parse query result row: missing column");
            }
            queryResult.push_back(std::move(rowResult));
        }
        return sol::nested<decltype(queryResult)>(std::move(queryResult));
    });

    m_luaState.script(INCLUA_BEGIN(char)
#include "serverluamodule.lua"
    INCLUA_END());
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
