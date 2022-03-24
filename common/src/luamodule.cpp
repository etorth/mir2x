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

#include <ctime>
#include <chrono>
#include <thread>
#include "log.hpp"
#include "uidf.hpp"
#include "luaf.hpp"
#include "totype.hpp"
#include "sysconst.hpp"
#include "fflerror.hpp"
#include "luamodule.hpp"
#include "raiitimer.hpp"
#include "dbcomid.hpp"
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

    m_luaState.script(str_printf("INVOP_TRADE  = %d", INVOP_TRADE ));
    m_luaState.script(str_printf("INVOP_SECURE = %d", INVOP_SECURE));
    m_luaState.script(str_printf("INVOP_REPAIR = %d", INVOP_REPAIR));

    m_luaState.script(str_printf("DIR_UP        = %d", DIR_UP       ));
    m_luaState.script(str_printf("DIR_UPRIGHT   = %d", DIR_UPRIGHT  ));
    m_luaState.script(str_printf("DIR_RIGHT     = %d", DIR_RIGHT    ));
    m_luaState.script(str_printf("DIR_DOWNRIGHT = %d", DIR_DOWNRIGHT));
    m_luaState.script(str_printf("DIR_DOWN      = %d", DIR_DOWN     ));
    m_luaState.script(str_printf("DIR_DOWNLEFT  = %d", DIR_DOWNLEFT ));
    m_luaState.script(str_printf("DIR_LEFT      = %d", DIR_LEFT     ));
    m_luaState.script(str_printf("DIR_UPLEFT    = %d", DIR_UPLEFT   ));

    m_luaState.script(str_printf("SYS_NPCINIT  = \"%s\"", SYS_NPCINIT ));
    m_luaState.script(str_printf("SYS_NPCDONE  = \"%s\"", SYS_NPCDONE ));
    m_luaState.script(str_printf("SYS_NPCQUERY = \"%s\"", SYS_NPCQUERY));
    m_luaState.script(str_printf("SYS_NPCERROR = \"%s\"", SYS_NPCERROR));
    m_luaState.script(str_printf("math.randomseed(%d)", to_d(hres_tstamp().to_nsec() % 1000000ULL)));

    m_luaState.set_function("addLogString", [this](sol::object logType, sol::object logInfo)
    {
        if(logType.is<int>() && logInfo.is<std::string>()){
            addLogString(logType.as<int>(), to_u8cstr(logInfo.as<std::string>()));
            return;
        }

        if(logType.is<int>()){
            addLogString(1, to_u8cstr(str_printf("Invalid argument: addLogString(%d, \"?\")", logType.as<int>())));
            return;
        }

        if(logInfo.is<std::string>()){
            addLogString(1, to_u8cstr(str_printf("Invalid argument: addLogString(?, \"%s\")", logInfo.as<std::string>().c_str())));
            return;
        }

        addLogString(1, u8"Invalid argument: addLogString(?, \"?\")");
    });

    m_luaState.set_function("getTime", [timer = hres_timer()]() -> int
    {
        return to_d(timer.diff_msec());
    });

    m_luaState.set_function("getAbsTime", []() -> int
    {
        return to_d(std::time(nullptr));
    });

    m_luaState.script(INCLUA_BEGIN(char)
#include "luamodule.lua"
    INCLUA_END());

    m_luaState.set_function("uidType", [](std::string uidString)
    {
        return uidf::getUIDType(uidf::toUIDEx(uidString));
    });

    m_luaState.set_function("isUID", [](std::string uidString)
    {
        try{
            uidf::toUIDEx(uidString);
            return true;
        }
        catch(...){
            return false;
        }
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

    m_luaState.set_function("getItemName", [](int itemID) -> std::string
    {
        if(const auto s = to_cstr(DBCOM_ITEMRECORD(itemID).name)){
            return s;
        }
        return "";
    });

    m_luaState.set_function("getItemID", [](std::string itemName) -> int
    {
        return DBCOM_ITEMID(to_u8cstr(itemName));
    });

    m_luaState.set_function("getMonsterName", [](int monsterID) -> std::string
    {
        if(const auto s = to_cstr(DBCOM_MONSTERRECORD(monsterID).name)){
            return s;
        }
        return "";
    });

    m_luaState.set_function("getMonsterID", [](std::string monsterName) -> int
    {
        return DBCOM_MONSTERID(to_u8cstr(monsterName));
    });

    m_luaState.set_function("getMapName", [](int mapID) -> std::string
    {
        return to_cstr(DBCOM_MAPRECORD(mapID).name);
    });

    m_luaState.set_function("getMapID", [](std::string mapName) -> int
    {
        return DBCOM_MAPID(to_u8cstr(mapName));
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

    m_luaState.set_function("scalarAsString", [this](sol::object obj) -> std::string
    {
        return luaf::buildBlob<sol::object>(obj);
    });

    m_luaState.set_function("convTableAsString", [this](sol::as_table_t<luaf::conv_table> convTable) -> std::string
    {
        return luaf::buildBlob<luaf::conv_table>(convTable.value());
    });

    m_luaState.set_function("scalarFromString", [this](std::string s, sol::this_state state)
    {
        return luaf::buildLuaObj(sol::state_view(state), s);
    });

    m_luaState.set_function("convTableFromString", [this](std::string s)
    {
        return luaf::buildLuaConvTable(s);
    });

    m_luaState.set_function("asKeyString", [this](std::string s) -> std::string
    {
        return luaf::asKeyString(s);
    });

    m_luaState.set_function("fromKeyString", [this](std::string s) -> std::string
    {
        return luaf::fromKeyString(s);
    });
}
