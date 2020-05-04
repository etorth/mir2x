/*
 * =====================================================================================
 *
 *       Filename: luamodule.cpp
 *        Created: 06/03/2017 20:26:17
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
#include <chrono>
#include <thread>
#include "sysconst.hpp"
#include "luamodule.hpp"
#include "dbcomrecord.hpp"

LuaModule::LuaModule()
    : m_LuaState()
{
    m_LuaState.open_libraries();
    m_LuaState.script(
            R"###( LOGTYPE_INFO      = 0    )###""\n"
            R"###( LOGTYPE_WARNING   = 1    )###""\n"
            R"###( LOGTYPE_FATAL     = 2    )###""\n"
            R"###( LOGTYPE_DEBUG     = 3    )###""\n");

    m_LuaState.script(str_printf("SYS_NPCINIT = \"%s\"", SYS_NPCINIT));
    m_LuaState.script(str_printf("SYS_NPCDONE = \"%s\"", SYS_NPCDONE));

    // get backtrace in lua
    // used in LuaModule to give location in the script

    m_LuaState.script(
            R"###( function getBackTraceLine()                                                  )###""\n"
            R"###(     local info = debug.getinfo(3, "Sl")                                      )###""\n"
            R"###(                                                                              )###""\n"
            R"###(     -- check if the backtracing info valid                                   )###""\n"
            R"###(     -- if not valid we return a empty string to addLog()                     )###""\n"
            R"###(                                                                              )###""\n"
            R"###(     if not info then                                                         )###""\n"
            R"###(         return ""                                                            )###""\n"
            R"###(     end                                                                      )###""\n"
            R"###(                                                                              )###""\n"
            R"###(     -- if it's invoked by a C function like:                                 )###""\n"
            R"###(     --     LuaModule["addLog"]("hello world")                                )###""\n"
            R"###(     -- then return "C_FUNC"                                                  )###""\n"
            R"###(                                                                              )###""\n"
            R"###(     if info.what == "C" then                                                 )###""\n"
            R"###(        return "C_FUNC"                                                       )###""\n"
            R"###(     end                                                                      )###""\n"
            R"###(                                                                              )###""\n"
            R"###(     -- invoked from a lua function                                           )###""\n"
            R"###(     -- return the invocation layer information                               )###""\n"
            R"###(                                                                              )###""\n"
            R"###(     return string.format("[%s]: %d", info.short_src, info.currentline)       )###""\n"
            R"###( end                                                                          )###""\n");

    // define _addLog_Raw() by LuaModule::addLog()
    // but don't call it since this is in constructor!
    // this function need lua::getBackTraceLine() to append the logInfo

    m_LuaState.set_function("addLog", [this](sol::object logType, sol::object logInfo)
    {
        if(logType.is<int>() && logInfo.is<std::string>()){
            addLog(logType.as<int>(), logInfo.as<std::string>().c_str());
            return;
        }

        if(logType.is<int>()){
            addLog(1, str_printf("Invalid addLog(%d, \"?\")", logType.as<int>()).c_str());
            return;
        }

        if(logInfo.is<std::string>()){
            addLog(1, str_printf("Invalid addLog(?, \"%s\")", logInfo.as<std::string>().c_str()).c_str());
            return;
        }

        addLog(1, "Invalid addLog(?, \"?\")");
        return;
    });

    m_LuaState.script(
            R"###( function addExtLog(logType, logInfo)                                 )###""\n"
            R"###(                                                                      )###""\n"
            R"###(     -- add type checking here                                        )###""\n"
            R"###(     -- need logType as int and logInfo as string                     )###""\n"
            R"###(                                                                      )###""\n"
            R"###(     if type(logType) == 'number' and type(logInfo) == 'string' then  )###""\n"
            R"###(         addLog(logType, getBackTraceLine() .. ': ' .. logInfo)       )###""\n"
            R"###(         return                                                       )###""\n"
            R"###(     end                                                              )###""\n"
            R"###(                                                                      )###""\n"
            R"###(     -- else we need to give warning                                  )###""\n"
            R"###(     addLog(1, 'addLog(logType: int, logInfo: string)')               )###""\n"
            R"###( end                                                                  )###""\n");

    m_LuaState.set_function("mapID2Name", [](int nMapID) -> std::string
    {
        return std::string(DBCOM_MAPRECORD((uint32_t)(nMapID)).Name);
    });

    m_LuaState.set_function("sleep", [](int nSleepMS)
    {
        if(nSleepMS > 0){
            std::this_thread::sleep_for(std::chrono::milliseconds(nSleepMS));
        }
    });

    m_LuaState.set_function("exit", [](int nExitCode)
    {
        std::exit(nExitCode);
    });
}
