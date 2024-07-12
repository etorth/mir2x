#include <ctime>
#include <chrono>
#include <thread>
#include "luaf.hpp"
#include "log.hpp"
#include "uidf.hpp"
#include "totype.hpp"
#include "hexstr.hpp"
#include "sysconst.hpp"
#include "fflerror.hpp"
#include "luamodule.hpp"
#include "raiitimer.hpp"
#include "dbcomid.hpp"
#include "cerealf.hpp"
#include "luaf.hpp"

LuaModule::LuaModule()
    : m_luaState()
    , m_replaceEnv(m_luaState, sol::create)
{
    m_luaState.open_libraries();
    execRawString(R"###(
        local _G = _G
        local error = error
        local coroutine = coroutine
        local _RSVD_NAME_G_sandbox = {}

        function getTLSTable()
            local threadId, inMainThread = coroutine.running()
            if inMainThread then
                error('call getTLSTable() in main thread')
            else
                if _RSVD_NAME_G_sandbox[threadId] == nil then
                    _RSVD_NAME_G_sandbox[threadId] = {}
                end
                return _RSVD_NAME_G_sandbox[threadId]
            end
        end

        local _RSVD_NAME_clearTLSTable = {}
        setmetatable(_RSVD_NAME_clearTLSTable, {__close = function()
            local threadId, inMainThread = coroutine.running()
            if inMainThread then
                error('call clearTLSTable() in main thread')
            else
                _RSVD_NAME_G_sandbox[threadId] = nil
            end
        end})

        function autoClearTLSTable()
            return _RSVD_NAME_clearTLSTable
        end

        _RSVD_NAME_replaceEnvMetaTable = {
            __index = function(_, key)
                local threadId, inMainThread = coroutine.running()
                if not inMainThread then
                    if _RSVD_NAME_G_sandbox[threadId] ~= nil and _RSVD_NAME_G_sandbox[threadId][key] ~= nil then
                        return _RSVD_NAME_G_sandbox[threadId][key]
                    end
                end
                return _G[key]
            end,

            __newindex = function(_, key, value)
                local threadId, inMainThread = coroutine.running()
                if inMainThread then
                    _G[key] = value
                else
                    if _RSVD_NAME_G_sandbox[threadId] == nil then
                        _RSVD_NAME_G_sandbox[threadId] = {}
                    end
                    _RSVD_NAME_G_sandbox[threadId][key] = value
                end
            end
        }
    )###");

    m_replaceEnv[sol::metatable_key] = sol::table(m_luaState["_RSVD_NAME_replaceEnvMetaTable"]);

    // idea from: https://blog.rubenwardy.com/2020/07/26/sol3-script-sandbox/
    // set replaceEnv as default environment, otherwise I don't know how to setup replaceEnv to thread/coroutine

    lua_rawgeti(m_luaState.lua_state(), LUA_REGISTRYINDEX, m_replaceEnv.registry_index());
    lua_rawseti(m_luaState.lua_state(), LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);

    execString("LOGTYPE_INFO    = 0");
    execString("LOGTYPE_WARNING = 1");
    execString("LOGTYPE_FATAL   = 2");
    execString("LOGTYPE_DEBUG   = 3");

    execString("UID_NONE  = %d", UID_NONE );
    execString("UID_BEGIN = %d", UID_BEGIN);
    execString("UID_COR   = %d", UID_COR  );
    execString("UID_MAP   = %d", UID_MAP  );
    execString("UID_NPC   = %d", UID_NPC  );
    execString("UID_MON   = %d", UID_MON  );
    execString("UID_PLY   = %d", UID_PLY  );
    execString("UID_RCV   = %d", UID_RCV  );
    execString("UID_END   = %d", UID_END  );

    execString("INVOP_TRADE  = %d", INVOP_TRADE );
    execString("INVOP_SECURE = %d", INVOP_SECURE);
    execString("INVOP_REPAIR = %d", INVOP_REPAIR);

    execString("DIR_UP        = %d", DIR_UP       );
    execString("DIR_UPRIGHT   = %d", DIR_UPRIGHT  );
    execString("DIR_RIGHT     = %d", DIR_RIGHT    );
    execString("DIR_DOWNRIGHT = %d", DIR_DOWNRIGHT);
    execString("DIR_DOWN      = %d", DIR_DOWN     );
    execString("DIR_DOWNLEFT  = %d", DIR_DOWNLEFT );
    execString("DIR_LEFT      = %d", DIR_LEFT     );
    execString("DIR_UPLEFT    = %d", DIR_UPLEFT   );

    execString("WLG_DRESS     = %d", WLG_DRESS   );
    execString("WLG_HELMET    = %d", WLG_HELMET  );
    execString("WLG_WEAPON    = %d", WLG_WEAPON  );
    execString("WLG_SHOES     = %d", WLG_SHOES   );
    execString("WLG_NECKLACE  = %d", WLG_NECKLACE);
    execString("WLG_ARMRING0  = %d", WLG_ARMRING0);
    execString("WLG_ARMRING1  = %d", WLG_ARMRING1);
    execString("WLG_RING0     = %d", WLG_RING0   );
    execString("WLG_RING1     = %d", WLG_RING1   );
    execString("WLG_TORCH     = %d", WLG_TORCH   );
    execString("WLG_CHARM     = %d", WLG_CHARM   );

    execString("SYS_DEBUG = %s", to_boolcstr(SYS_DEBUG));
    execString("SYS_GOLDNAME = %s", luaf::quotedLuaString(SYS_GOLDNAME).c_str());

    execString("SYS_LABEL = %s", luaf::quotedLuaString(SYS_LABEL).c_str());
    execString("SYS_ENTER = %s", luaf::quotedLuaString(SYS_ENTER).c_str());
    execString("SYS_DONE  = %s", luaf::quotedLuaString(SYS_DONE ).c_str());
    execString("SYS_EXIT  = %s", luaf::quotedLuaString(SYS_EXIT ).c_str());
    execString("SYS_ABORT = %s", luaf::quotedLuaString(SYS_ABORT).c_str());

    execString("SYS_POSINF = %s", str_quoted(SYS_POSINF).c_str());
    execString("SYS_NEGINF = %s", str_quoted(SYS_NEGINF).c_str());

    execString(R"###(
        -- lua system quest variables
        -- access as table

        SYS_QUESTFIELD = {

             VARS = %s,
            FLAGS = %s,
            STATE = %s,

            TEAM = {
                LEADER     = %s,
                MEMBERLIST = %s,
                ROLELIST   = %s,
            },
        }
    )###",

    luaf::quotedLuaString(SYS_QUESTFIELD::VARS            ).c_str(),
    luaf::quotedLuaString(SYS_QUESTFIELD::FLAGS           ).c_str(),
    luaf::quotedLuaString(SYS_QUESTFIELD::STATE           ).c_str(),
    luaf::quotedLuaString(SYS_QUESTFIELD::TEAM::LEADER    ).c_str(),
    luaf::quotedLuaString(SYS_QUESTFIELD::TEAM::MEMBERLIST).c_str(),
    luaf::quotedLuaString(SYS_QUESTFIELD::TEAM::ROLELIST  ).c_str());


    execString("SYS_NPCERROR = \'%s\'", SYS_NPCERROR);

    execString("SYS_EXECDONE   = %s", str_quoted(SYS_EXECDONE  ).c_str());
    execString("SYS_EXECCLOSE  = %s", str_quoted(SYS_EXECCLOSE ).c_str());
    execString("SYS_EXECBADUID = %s", str_quoted(SYS_EXECBADUID).c_str());

    execString("SYS_FLAGVAL = %s", str_quoted(SYS_FLAGVAL).c_str());

    execString("SYS_EPDEF = \'%s\'", SYS_EPDEF);
    execString("SYS_EPUID = \'%s\'", SYS_EPUID);
    execString("SYS_EPQST = \'%s\'", SYS_EPQST);

    execString("SYS_ON_BEGIN    = %d", SYS_ON_BEGIN);
    execString("SYS_ON_GAINEXP  = %d", SYS_ON_GAINEXP);
    execString("SYS_ON_GAINITEM = %d", SYS_ON_GAINITEM);
    execString("SYS_ON_GAINGOLD = %d", SYS_ON_GAINGOLD);
    execString("SYS_ON_ONLINE   = %d", SYS_ON_ONLINE);
    execString("SYS_ON_OFFLINE  = %d", SYS_ON_OFFLINE);
    execString("SYS_ON_LEVELUP  = %d", SYS_ON_LEVELUP);
    execString("SYS_ON_KILL     = %d", SYS_ON_KILL);
    execString("SYS_ON_TEAMUP   = %d", SYS_ON_TEAMUP);
    execString("SYS_ON_TEAMDOWN = %d", SYS_ON_TEAMDOWN);
    execString("SYS_ON_APPEAR   = %d", SYS_ON_APPEAR);
    execString("SYS_ON_END      = %d", SYS_ON_END);

    execString("SYS_COOP        = \'%s\'", SYS_COOP);
    execString("SYS_QSTFSM      = \'%s\'", SYS_QSTFSM);
    execString("SYS_HIDE        = \'%s\'", SYS_HIDE);
    execString("SYS_CHECKACTIVE = \'%s\'", SYS_CHECKACTIVE);
    execString("SYS_ALLOWREDNAME = \'%s\'", SYS_ALLOWREDNAME);

    execString("math.randomseed(%d)", to_d(hres_tstamp().to_nsec() % 1000000ULL));

    bindFunction("debugAttach", [this]()
    {
        addLogString(1, to_u8cstr(str_printf("Waiting for debugger to attach pid %llu", to_llu(getpid()))));
    });

    bindFunction("addLogString", [this](sol::object logType, sol::object logInfo)
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

    bindFunction("getTime", [timer = hres_timer()]() -> double
    {
        return timer.diff_msecf();
    });

    bindFunction("getNanoTstamp", []() -> uint64_t
    {
        return hres_tstamp().to_nsec();
    });

    bindFunction("getAbsTime", []() -> int
    {
        return to_d(std::time(nullptr));
    });

    bindFunction("getUIDType", [](uint64_t uid)
    {
        return uidf::getUIDType(uid);
    });

    bindFunction("isPlayer", [](uint64_t uid)
    {
        return uidf::isPlayer(uid);
    });

    bindFunction("isQuest", [](uint64_t uid)
    {
        return uidf::isQuest(uid);
    });

    bindFunction("isNPChar", [](uint64_t uid)
    {
        return uidf::isNPChar(uid);
    });

    bindFunction("isMonster", [](uint64_t uid)
    {
        return uidf::isMonster(uid);
    });

    bindFunction("sleep", [](int nSleepMS)
    {
        if(nSleepMS > 0){
            std::this_thread::sleep_for(std::chrono::milliseconds(nSleepMS));
        }
    });

    bindFunction("exit", [](int exitCode)
    {
        std::exit(exitCode);
    });

    bindFunction("getItemName", [](sol::object arg, sol::this_state s) -> sol::object
    {
        if(arg.is<lua_Integer>()){
            if(const auto name = DBCOM_ITEMRECORD(arg.as<lua_Integer>()).name; str_haschar(name)){
                return sol::object(sol::state_view(s), sol::in_place_type<std::string>, std::string(to_cstr(name)));
            }
            return sol::make_object(sol::state_view(s), sol::nil);
        }
        else if(arg == sol::nil){
            return sol::make_object(sol::state_view(s), sol::nil);
        }
        else{
            throw fflerror("invalid argument type: %s", luaf::luaObjTypeString(arg).c_str());
        }
    });

    bindFunction("getItemID", [](std::string itemName) -> int
    {
        return DBCOM_ITEMID(to_u8cstr(itemName));
    });

    bindFunction("getMonsterName", [](int monsterID, sol::this_state s) -> sol::object
    {
        sol::state_view sv(s);
        if(const auto name = DBCOM_MONSTERRECORD(monsterID).name; str_haschar(name)){
            return sol::object(sv, sol::in_place_type<std::string>, std::string(to_cstr(name)));
        }
        return sol::make_object(sv, sol::nil);
    });

    bindFunction("getMonsterID", [](std::string monsterName) -> int
    {
        return DBCOM_MONSTERID(to_u8cstr(monsterName));
    });

    bindFunction("getMapName", [](int mapID) -> std::string
    {
        return to_cstr(DBCOM_MAPRECORD(mapID).name);
    });

    bindFunction("getMapID", [](std::string mapName) -> int
    {
        return DBCOM_MAPID(to_u8cstr(mapName));
    });

    bindFunction("hexString", [](std::string s)
    {
        char buf[8];
        std::string hexStr;

        for(const auto ch: s){
            hexStr.insert(hexStr.end(), hexstr::to_string((unsigned char)(ch), buf, false), (const char *)(buf) + 2);
        }
        return hexStr;
    });

    bindFunction("randString", [this](sol::variadic_args args) -> std::string
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

    bindFunction("strAny", [this](sol::object obj) -> std::string
    {
        return str_any(luaf::buildLuaVar(obj));
    });

    bindFunction("strUpper", [this](std::string s) -> std::string
    {
        return str_toupper(s);
    });

    bindFunction("strLower", [this](std::string s) -> std::string
    {
        return str_tolower(s);
    });

    bindFunction("base64Encode", [this](sol::object obj) -> std::string
    {
        return cerealf::base64_serialize(luaf::buildLuaVar(obj));
    });

    bindFunction("base64Decode", [](std::string data, sol::this_state s) -> sol::object
    {
        return luaf::buildLuaObj(sol::state_view(s), cerealf::base64_deserialize<luaf::luaVar>(data));
    });

    bindFunction("quotedLuaString", [](std::string s) -> std::string
    {
        // make follow two print identical results:
        //
        // local s = 'string_literal_xxxx'
        // print(s)
        // print(string.format([[%s]], quotedLuaString(s)))

        return luaf::quotedLuaString(s);
    });

    m_luaState.script(BEGIN_LUAINC(char)
#include "luamodule.lua"
    END_LUAINC());
}

bool LuaModule::pfrCheck(const sol::protected_function_result &pfr, const std::function<void(const std::string &)> &errDrainFunc)
{
    if(pfr.valid()){
        return true;
    }

    const sol::error err = pfr;
    std::stringstream errStream(err.what());

    std::string errStr;
    while(std::getline(errStream, errStr, '\n')){
        if(errDrainFunc){
            errDrainFunc(errStr);
        }
        else{
            addLogString(Log::LOGTYPEV_WARNING, to_u8cstr(errStr));
        }
    }
    return false;
}
