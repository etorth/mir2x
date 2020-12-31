/*
 * =====================================================================================
 *
 *       Filename: npchar.cpp
 *        Created: 04/12/2020 16:01:51
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

#include <cstdint>
#include "uidf.hpp"
#include "npchar.hpp"
#include "totype.hpp"
#include "filesys.hpp"
#include "dbcomid.hpp"
#include "cerealf.hpp"
#include "fflerror.hpp"
#include "serdesmsg.hpp"
#include "friendtype.hpp"
#include "monoserver.hpp"
#include "dbcomrecord.hpp"
#include "serverconfigurewindow.hpp"

extern MonoServer *g_monoServer;
extern ServerConfigureWindow *g_serverConfigureWindow;

NPChar::LuaNPCModule::LuaNPCModule(NPChar *npc)
    : ServerLuaModule()
{
    m_luaState.set_function("getUID", [npc]() -> std::string
    {
        return std::to_string(npc->rawUID());
    });

    m_luaState.set_function("getUIDString", [](std::string uidString) -> std::string
    {
        return uidf::getUIDString(uidf::toUID(uidString));
    });

    m_luaState.set_function("getNPCName", [npc]() -> std::string
    {
        return std::string(to_cstr(DBCOM_NPCRECORD(uidf::getNPCID(npc->rawUID())).name));
    });

    m_luaState.set_function("getNPCFullName", [npc]() -> std::string
    {
        return std::string(to_cstr(DBCOM_MAPRECORD(uidf::getMapID(npc->m_map->UID())).name)) + "." + std::string(to_cstr(DBCOM_NPCRECORD(uidf::getNPCID(npc->rawUID())).name));
    });

    m_luaState.set_function("sendSell", [npc, this](std::string uidString, std::string listName)
    {
        if(const sol::object checkObj = m_luaState[listName]; checkObj == sol::nil){
            throw fflerror("lua script has no variable defined: %s", to_cstr(listName));
        }

        std::vector<std::string> itemList;
        sol::table luaTable = m_luaState[listName];

        for(const auto &p: luaTable){
            itemList.push_back(p.second.as<std::string>());
        }
        npc->sendSell(uidf::toUIDEx(uidString), itemList);
    });

    m_luaState.set_function("sendQuery", [npc](std::string s, std::string uidString, std::string query)
    {
        npc->sendQuery(uidf::toUIDEx(s), uidf::toUIDEx(uidString), query);
    });

    m_luaState.set_function("sayXML", [npc, this](std::string uidString, std::string xmlString)
    {
        const uint64_t uid = [&uidString]() -> uint64_t
        {
            try{
                return std::stoull(uidString);
            }
            catch(...){
                //
            }
            return 0;
        }();

        if(uid){
            npc->sendXMLLayout(uid, xmlString.c_str());
        }

        else{
            addLog(0, u8"invalid UID: 0");
        }
    });

    m_luaState.set_function("pollEvent", [this](std::string uidString)
    {
        const uint64_t uid = [&uidString]() -> uint64_t
        {
            try{
                return std::stoull(uidString);
            }
            catch(...){
                //
            }
            return 0;
        }();

        return sol::as_returns([uid, this]() -> std::vector<std::string>
        {
            if(auto p = m_sessionList.find(uid); p != m_sessionList.end()){
                if(p->second.event.empty()){
                    return {};
                }

                if(p->second.value.empty()){
                    return {std::move(p->second.event)};
                }
                return {std::move(p->second.event), std::move(p->second.value)};
            }
            throw fflerror("can't find session for UID = %llu", to_llu(uid));
        }());
    });

    m_luaState.script
    (
        R"###( function has_processNPCEvent(verbose, event)                                                                                  )###""\n"
        R"###(     verbose = verbose or false                                                                                                )###""\n"
        R"###(     if type(verbose) ~= 'boolean' then                                                                                        )###""\n"
        R"###(         verbose = false                                                                                                       )###""\n"
        R"###(         addLog(LOGTYPE_WARNING, 'parmeter verbose is not boolean type, assumed false')                                        )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###(     if type(event) ~= 'string' then                                                                                           )###""\n"
        R"###(         event = nil                                                                                                           )###""\n"
        R"###(         addLog(LOGTYPE_WARNING, 'parmeter event is not string type, ignored')                                                 )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###(     if not processNPCEvent then                                                                                               )###""\n"
        R"###(         if verbose then                                                                                                       )###""\n"
        R"###(             addLog(LOGTYPE_WARNING, "NPC " .. getNPCFullName() .. ": processNPCEvent is not defined")                         )###""\n"
        R"###(         end                                                                                                                   )###""\n"
        R"###(         return false                                                                                                          )###""\n"
        R"###(     elseif type(processNPCEvent) ~= 'table' then                                                                              )###""\n"
        R"###(         if verbose then                                                                                                       )###""\n"
        R"###(             addLog(LOGTYPE_WARNING, "NPC " .. getNPCFullName() .. ": processNPCEvent is not a function table")                )###""\n"
        R"###(         end                                                                                                                   )###""\n"
        R"###(         return false                                                                                                          )###""\n"
        R"###(     else                                                                                                                      )###""\n"
        R"###(         local count = 0                                                                                                       )###""\n"
        R"###(         for _ in pairs(processNPCEvent) do                                                                                    )###""\n"
        R"###(             -- here for each entry we can check if the key is string and value is function type                               )###""\n"
        R"###(             -- but can possibly be OK if the event is not triggered                                                           )###""\n"
        R"###(             count = count + 1                                                                                                 )###""\n"
        R"###(         end                                                                                                                   )###""\n"
        R"###(         if count == 0 then                                                                                                    )###""\n"
        R"###(             if verbose then                                                                                                   )###""\n"
        R"###(                 addLog(LOGTYPE_WARNING, "NPC " .. getNPCFullName() .. ": processNPCEvent is empty")                           )###""\n"
        R"###(             end                                                                                                               )###""\n"
        R"###(             return false                                                                                                      )###""\n"
        R"###(         end                                                                                                                   )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###(         if not event then                                                                                                     )###""\n"
        R"###(             return true                                                                                                       )###""\n"
        R"###(         end                                                                                                                   )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###(         if type(processNPCEvent[event]) ~= 'function' then                                                                    )###""\n"
        R"###(             if verbose then                                                                                                   )###""\n"
        R"###(                 addLog(LOGTYPE_WARNING, "NPC " .. getNPCFullName() .. ": processNPCEvent[" .. event .. "] is not a function") )###""\n"
        R"###(             end                                                                                                               )###""\n"
        R"###(             return false                                                                                                      )###""\n"
        R"###(         end                                                                                                                   )###""\n"
        R"###(         return true                                                                                                           )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function queryName(s, uid)                                                                                                    )###""\n"
        R"###(     sendQuery(s, uid or s, 'NAME')                                                                                            )###""\n"
        R"###(     local event, value = waitEvent(s)                                                                                         )###""\n"
        R"###(     if event ~= SYS_NPCQUERY then                                                                                             )###""\n"
        R"###(         addLog(LOGTYPE_WARNING, 'Wait event as SYS_NPCQUERY but get ' .. tostring(event))                                     )###""\n"
        R"###(         return nil                                                                                                            )###""\n"
        R"###(     else                                                                                                                      )###""\n"
        R"###(         return value                                                                                                          )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function queryLevel(s, uid)                                                                                                   )###""\n"
        R"###(     sendQuery(s, uid or s, 'LEVEL')                                                                                           )###""\n"
        R"###(     local event, value = waitEvent(s)                                                                                         )###""\n"
        R"###(     if event ~= SYS_NPCQUERY then                                                                                             )###""\n"
        R"###(         addLog(LOGTYPE_WARNING, 'Wait event as SYS_NPCQUERY but get ' .. tostring(event))                                     )###""\n"
        R"###(         return nil                                                                                                            )###""\n"
        R"###(     else                                                                                                                      )###""\n"
        R"###(         return tonumber(value)                                                                                                )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function queryGold(s, uid)                                                                                                    )###""\n"
        R"###(     sendQuery(s, uid or s, 'GOLD')                                                                                            )###""\n"
        R"###(     local event, value = waitEvent(s)                                                                                         )###""\n"
        R"###(     if event ~= SYS_NPCQUERY then                                                                                             )###""\n"
        R"###(         addLog(LOGTYPE_WARNING, 'Wait event as SYS_NPCQUERY but get ' .. tostring(event))                                     )###""\n"
        R"###(         return nil                                                                                                            )###""\n"
        R"###(     else                                                                                                                      )###""\n"
        R"###(         return tonumber(value)                                                                                                )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function postSell(uid, sell)                                                                                                  )###""\n"
        R"###(     if type(sell) ~= 'table' then                                                                                             )###""\n"
        R"###(         addLog(LOGTYPE_WARNING, 'postSell() expectes a table but get ' .. type(sell))                                         )###""\n"
        R"###(         return                                                                                                                )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###(     local randListName = 'varName_' .. randString(12)                                                                         )###""\n"
        R"###(     _G[randListName] = sell                                                                                                   )###""\n"
        R"###(     sendSell(uid, randListName)                                                                                               )###""\n"
        R"###(     _G[randListName] = nil                                                                                                    )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function waitEvent(uid)                                                                                                       )###""\n"
        R"###(     while true do                                                                                                             )###""\n"
        R"###(         local event, value = pollEvent(uid)                                                                                   )###""\n"
        R"###(         if event then                                                                                                         )###""\n"
        R"###(             return event, value                                                                                               )###""\n"
        R"###(         end                                                                                                                   )###""\n"
        R"###(         coroutine.yield()                                                                                                     )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function main(uid)                                                                                                            )###""\n"
        R"###(     while true do                                                                                                             )###""\n"
        R"###(         local event, value = waitEvent(uid)                                                                                   )###""\n"
        R"###(         if has_processNPCEvent(false, event) then                                                                             )###""\n"
        R"###(             processNPCEvent[event](uid, value)                                                                                )###""\n"
        R"###(         else                                                                                                                  )###""\n"
        R"###(             -- don't exit this loop                                                                                           )###""\n"
        R"###(             -- always consume the event no matter if the NPC can handle it                                                    )###""\n"
        R"###(             sayXML(uid, string.format(                                                                                        )###""\n"
        R"###(             [[                                                                                                                )###""\n"
        R"###(                 <layout>                                                                                                      )###""\n"
        R"###(                     <par>我听不懂你在说什么...</par>                                                                          )###""\n"
        R"###(                     <par><event id="%s">关闭</event></par>                                                                    )###""\n"
        R"###(                 </layout>                                                                                                     )###""\n"
        R"###(             ]], SYS_NPCDONE))                                                                                                 )###""\n"
        R"###(         end                                                                                                                   )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###( end                                                                                                                           )###""\n");

    m_luaState.script_file([npc]() -> std::string
    {
        const auto scriptPath = []() -> std::string
        {
            if(const auto cfgScriptPath = g_serverConfigureWindow->getScriptPath(); !cfgScriptPath.empty()){
                return cfgScriptPath + "/npc";
            }
            return std::string("script/npc");
        }();

        const auto mapName = to_cstr(DBCOM_MAPRECORD(uidf::getMapID(npc->m_map->UID())).name);
        const auto npcName = to_cstr(DBCOM_NPCRECORD(uidf::getNPCID(npc->rawUID())).name);

        for(const auto &fileName:
        {
            str_printf("%s/%s.%s.lua",           scriptPath.c_str(), mapName, npcName),
            str_printf("%s/default/%s.lua",      scriptPath.c_str(), npcName),
            str_printf("%s/default/default.lua", scriptPath.c_str()),
        }){
            if(filesys::hasFile(fileName.c_str())){
                return fileName;
            }
        }
        throw fflerror("no available script for NPC %s.%s", mapName, npcName);
    }());

    // NPC script has no state
    // after source it finishes the script, not yield

    m_luaState.script
    (
        R"###( -- do the first sanity check here                               )###""\n"
        R"###( -- last in main call we also check it but with verbose disabled )###""\n"
        R"###( has_processNPCEvent(true, SYS_NPCINIT)                          )###""\n");
}

void NPChar::LuaNPCModule::setEvent(uint64_t uid, std::string event, std::string value)
{
    if(!(uid && !event.empty())){
        throw fflerror("invalid argument: uid = %llu, event = %s, value = %s", to_llu(uid), to_cstr(event), to_cstr(value));
    }

    if(event == SYS_NPCDONE){
        m_sessionList.erase(uid);
        return;
    }

    const auto fnCheckCOResult = [this](const auto &result)
    {
        if(result.valid()){
            return;
        }

        const sol::error err = result;
        std::stringstream errStream(err.what());

        std::string errLine;
        while(std::getline(errStream, errLine, '\n')){
            addLog(1, to_u8cstr(errLine));
        }
    };

    auto p = m_sessionList.find(uid);
    if(p == m_sessionList.end()){
        if(event != SYS_NPCINIT){
            throw fflerror("get script event %s while no communication initialized", event.c_str());
        }

        LuaNPCModule::LuaNPCSession session;
        session.uid = uid;
        session.module = this;

        session.co_handler.runner = sol::thread::create(getLuaState().lua_state());
        session.co_handler.callback = sol::state_view(session.co_handler.runner.state())["main"];

        p = m_sessionList.insert({uid, std::move(session)}).first;

        p->second.event.clear();
        p->second.value.clear();
        const auto result = p->second.co_handler.callback(std::to_string(uid));
        fnCheckCOResult(result);
    }

    if(p->second.uid != uid){
        throw fflerror("invalid session: key = %llu, value.key = %llu", to_llu(uid), to_llu(p->second.uid));
    }

    if(!p->second.co_handler.callback){
        throw fflerror("lua coroutine is not callable");
    }

    // clear the event
    // call the coroutine to make it stuck at pollEvent()

    p->second.event = std::move(event);
    p->second.value = std::move(value);

    const auto result = p->second.co_handler.callback();
    fnCheckCOResult(result);
}

NPChar::NPChar(uint16_t npcId, ServiceCore *core, ServerMap *serverMap, int mapX, int mapY)
    : CharObject(core, serverMap, uidf::buildNPCUID(npcId), mapX, mapY, DIR_NONE)
{
    // LuaNPCModule(this) can access ``this"
    // when constructing LuaNPCModule we need to confirm ``this" is ready
    m_luaModulePtr = std::make_unique<NPChar::LuaNPCModule>(this);
}

bool NPChar::update()
{
    return true;
}

bool NPChar::InRange(int, int, int)
{
    return true;
}

void NPChar::reportCO(uint64_t)
{
}

bool NPChar::DCValid(int, bool)
{
    return true;
}

DamageNode NPChar::GetAttackDamage(int)
{
    return {};
}

bool NPChar::StruckDamage(const DamageNode &)
{
    return true;
}

void NPChar::checkFriend(uint64_t, std::function<void(int)> fnOP)
{
    fnOP(FT_NEUTRAL);
}

bool NPChar::goDie()
{
    return true;
}

bool NPChar::goGhost()
{
    return true;
}

void NPChar::sendSell(uint64_t uid, const std::vector<std::string> &itemList)
{
    AMNPCSell amNPCS;
    std::memset(&amNPCS, 0, sizeof(amNPCS));

    amNPCS.ptr = new std::string(cerealf::serialize(SDNPCSell
    {
        .npcUID = uid,
        .itemList = [&itemList]()
        {
            std::vector<uint32_t> itemIDList;
            for(const auto &itemName: itemList){
                if(const uint32_t itemID = DBCOM_ITEMID(to_u8cstr(itemName))){
                    itemIDList.push_back(itemID);
                }
                else{
                    g_monoServer->addLog(LOGTYPE_WARNING, "invalid NPC selling item: %s", to_cstr(itemName));
                }
            }
            return itemIDList;
        }()
    }, true));
    m_actorPod->forward(uid, {MPK_NPCSELL, amNPCS});
}

void NPChar::sendQuery(uint64_t s, uint64_t uid, const std::string &query)
{
    AMNPCQuery amNPCQ;
    std::memset(&amNPCQ, 0, sizeof(amNPCQ));

    if(query.size() >= sizeof(amNPCQ.query)){
        throw fflerror("query name is too long: %s", query.c_str());
    }

    std::strcpy(amNPCQ.query, query.c_str());
    m_actorPod->forward(uid, {MPK_NPCQUERY, amNPCQ}, [s, this](const MessagePack &mpk)
    {
        switch(mpk.Type()){
            case MPK_NPCEVENT:
                {
                    const auto amNPCE = mpk.conv<AMNPCEvent>();
                    m_luaModulePtr->setEvent(s, amNPCE.event, amNPCE.value);
                    return;
                }
            default:
                {
                    m_luaModulePtr->close(s);
                    return;
                }
        }
    });
}

void NPChar::sendXMLLayout(uint64_t uid, const char *xmlString)
{
    if(!xmlString){
        throw fflerror("invalid xmlString: %s", to_cstr(xmlString));
    }

    AMNPCXMLLayout amNPCXMLL;
    std::memset(&amNPCXMLL, 0, sizeof(amNPCXMLL));

    amNPCXMLL.xmlLayout = new std::string(xmlString);
    m_actorPod->forward(uid, {MPK_NPCXMLLAYOUT, amNPCXMLL});
}

void NPChar::operateAM(const MessagePack &mpk)
{
    switch(mpk.Type()){
        case MPK_OFFLINE:
        case MPK_METRONOME:
            {
                break;
            }
        case MPK_ACTION:
            {
                on_MPK_ACTION(mpk);
                break;
            }
        case MPK_NPCEVENT:
            {
                on_MPK_NPCEVENT(mpk);
                break;
            }
        case MPK_NOTIFYNEWCO:
            {
                on_MPK_NOTIFYNEWCO(mpk);
                break;
            }
        case MPK_QUERYCORECORD:
            {
                on_MPK_QUERYCORECORD(mpk);
                break;
            }
        case MPK_QUERYLOCATION:
            {
                on_MPK_QUERYLOCATION(mpk);
                break;
            }
        case MPK_BADACTORPOD:
            {
                on_MPK_BADACTORPOD(mpk);
                break;
            }
        default:
            {
                throw fflerror("unsupported message: %s", mpkName(mpk.Type()));
            }
    }
}
