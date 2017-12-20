/*
 * =====================================================================================
 *
 *       Filename: luamodule.cpp
 *        Created: 06/03/2017 20:26:17
 *  Last Modified: 12/20/2017 02:40:50
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
#include <chrono>
#include <thread>
#include "luamodule.hpp"

LuaModule::LuaModule()
    : m_LuaState()
{
    m_LuaState.open_libraries(sol::lib::base);
    m_LuaState.open_libraries(sol::lib::math);
    m_LuaState.open_libraries(sol::lib::debug);
    m_LuaState.open_libraries(sol::lib::string);

    m_LuaState.script(
            R"###( LOGTYPE_INFO      = 0    )###""\n"
            R"###( LOGTYPE_WARNING   = 1    )###""\n"
            R"###( LOGTYPE_FATAL     = 2    )###""\n"
            R"###( LOGTYPE_DEBUG     = 3    )###""\n");

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

    m_LuaState.set_function("_addLog_Raw", [this](sol::object stLogType, sol::object stLogInfo)
    {
        if(stLogType.is<int>() && stLogInfo.is<std::string>()){
            addLog(stLogType.as<int>(), stLogInfo.as<std::string>().c_str());
            return;
        }

        if(stLogType.is<int>()){
            std::string szWarnInfo;
            szWarnInfo += "Invalid addLog(";
            szWarnInfo += std::to_string(stLogType.as<int>());
            szWarnInfo += ", \"?\")";

            addLog(1, szWarnInfo.c_str());
            return;
        }

        if(stLogInfo.is<std::string>()){
            std::string szWarnInfo;
            szWarnInfo += "Invalid addLog(?, \"";
            szWarnInfo += stLogInfo.as<std::string>();
            szWarnInfo += "\")";

            addLog(1, szWarnInfo.c_str());
            return;
        }

        {
            addLog(1, "Invalid addLog(?, ?)");
            return;
        }
    });

    m_LuaState.script(
            R"###( function addLog(logType, logInfo)                                    )###""\n"
            R"###(                                                                      )###""\n"
            R"###(     -- add type checking here                                        )###""\n"
            R"###(     -- need logType as int and logInfo as string                     )###""\n"
            R"###(                                                                      )###""\n"
            R"###(     if type(logType) == "number" and type(logInfo) == "string" then  )###""\n"
            R"###(         _addLog_Raw(logType, getBackTraceLine() .. ": " .. logInfo)  )###""\n"
            R"###(         return                                                       )###""\n"
            R"###(     end                                                              )###""\n"
            R"###(                                                                      )###""\n"
            R"###(     -- else we need to give warning                                  )###""\n"
            R"###(     _addLog_Raw(1, "addLog(logType: int, logInfo: string)")          )###""\n"
            R"###( end                                                                  )###""\n");

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
