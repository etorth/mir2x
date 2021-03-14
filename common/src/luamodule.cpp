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

#include <chrono>
#include <thread>
#include "log.hpp"
#include "uidf.hpp"
#include "totype.hpp"
#include "sysconst.hpp"
#include "fflerror.hpp"
#include "luamodule.hpp"
#include "dbcomrecord.hpp"

LuaModule::LuaModule()
    : m_luaState()
{
    m_luaState.open_libraries();
    m_luaState.script(str_printf("UID_ERR = %d", UID_ERR));
    m_luaState.script(str_printf("UID_COR = %d", UID_COR));
    m_luaState.script(str_printf("UID_NPC = %d", UID_NPC));
    m_luaState.script(str_printf("UID_MAP = %d", UID_MAP));
    m_luaState.script(str_printf("UID_PLY = %d", UID_PLY));
    m_luaState.script(str_printf("UID_MON = %d", UID_MON));
    m_luaState.script(str_printf("UID_ETC = %d", UID_ETC));
    m_luaState.script(str_printf("UID_INN = %d", UID_INN));

    m_luaState.script(str_printf("SYS_NPCINIT  = \"%s\"", SYS_NPCINIT ));
    m_luaState.script(str_printf("SYS_NPCDONE  = \"%s\"", SYS_NPCDONE ));
    m_luaState.script(str_printf("SYS_NPCQUERY = \"%s\"", SYS_NPCQUERY));
    m_luaState.script(str_printf("SYS_NPCERROR = \"%s\"", SYS_NPCERROR));

    m_luaState.set_function("addLogString", [this](sol::object logType, sol::object logInfo)
    {
        if(logType.is<int>() && logInfo.is<std::string>()){
            addLog(logType.as<int>(), to_u8cstr(logInfo.as<std::string>()));
            return;
        }

        if(logType.is<int>()){
            addLog(1, to_u8cstr(str_printf("Invalid argument: addLogString(%d, \"?\")", logType.as<int>())));
            return;
        }

        if(logInfo.is<std::string>()){
            addLog(1, to_u8cstr(str_printf("Invalid argument: addLogString(?, \"%s\")", logInfo.as<std::string>().c_str())));
            return;
        }

        addLog(1, u8"Invalid argument: addLogString(?, \"?\")");
    });

    m_luaState.script(INCLUA_BEGIN(char)
#include "luamodule.lua"
    INCLUA_END());

    m_luaState.set_function("uidType", [](std::string uidString)
    {
        return uidf::getUIDType(uidf::toUIDEx(uidString));
    });

    m_luaState.set_function("mapID2Name", [](int nMapID) -> std::string
    {
        return std::string(reinterpret_cast<const char *>(DBCOM_MAPRECORD(to_u32(nMapID)).name));
    });

    m_luaState.set_function("sleep", [](int nSleepMS)
    {
        if(nSleepMS > 0){
            std::this_thread::sleep_for(std::chrono::milliseconds(nSleepMS));
        }
    });

    m_luaState.set_function("exit", [](int nExitCode)
    {
        std::exit(nExitCode);
    });

    m_luaState.set_function("randString", [this](sol::variadic_args args) -> std::string
    {
        // generate random string
        // for debug purpose of utf8 layout board

        int length = 0;
        std::string alphabet;

        const std::vector<sol::object> argList(args.begin(), args.end());
        switch(argList.size()){
            case 1:
                {
                    if(!argList[0].is<int>()){
                        throw fflerror("Invalid argument: randString(length: int, [alphabet: string])");
                    }

                    length = argList[0].as<int>();
                    alphabet = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
                    break;
                }

            case 2:
                {
                    if(!(argList[0].is<int>() && argList[1].is<std::string>())){
                        throw fflerror("Invalid argument: randString(length: int, [alphabet: string])");
                    }

                    length = argList[0].as<int>();
                    alphabet = argList[1].as<std::string>();
                    break;
                }
            default:
                {
                    throw fflerror("Invalid argument: randString(length: int, [alphabet: string])");
                }
        }

        if(length < 0 || alphabet.empty()){
            const auto reportAlphabet = [&alphabet]() -> std::string
            {
                if(alphabet.empty()){
                    return "(empty)";
                }

                if(alphabet.length() < 5){
                    return alphabet;
                }
                return alphabet.substr(0, 3) + "...";
            }();
            throw fflerror("Invalid argument: randString(length = %d, alphabe = \'%s\')", length, reportAlphabet.c_str());
        }

        std::string result;
        for(int i = 0; i < length; ++i){
            result.push_back(alphabet[std::rand() % alphabet.length()]);
        }
        return result;
    });
}
