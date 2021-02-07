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

    m_luaState.set_function("sendSessionQuery", [npc](std::string sessionUID, std::string uidString, std::string query)
    {
        npc->sendQuery(uidf::toUIDEx(sessionUID), uidf::toUIDEx(uidString), query);
    });

    m_luaState.set_function("getSellItemList", [](sol::this_state luaPtr)
    {
        std::vector<std::string> itemNameList;
        for(uint32_t i = 1; i < 1000; ++i){
            if(const auto &ir = DBCOM_ITEMRECORD(i)){
                itemNameList.push_back(to_cstr(ir.name));
            }
        }
        return sol::make_object(sol::state_view(luaPtr), itemNameList);
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
            npc->sendXMLLayout(uid, std::move(xmlString));
        }

        else{
            addLog(0, u8"invalid UID: 0");
        }
    });

    m_luaState.set_function("pollSessionEvent", [this](std::string sessionUID)
    {
        const uint64_t uid = [&sessionUID]() -> uint64_t
        {
            try{
                return std::stoull(sessionUID);
            }
            catch(...){
                //
            }
            return 0;
        }();

        return sol::as_returns([uid, this]() -> std::vector<std::string>
        {
            if(auto p = m_sessionList.find(uid); p != m_sessionList.end()){
                const auto fromUID = p->second.from;
                p->second.from = 0;

                if(!fromUID){
                    return {};
                }

                if(p->second.event.empty()){
                    throw fflerror("detected uid %llu with empty event", to_llu(fromUID));
                }


                if(p->second.value.empty()){
                    return {std::to_string(fromUID), std::move(p->second.event)};
                }
                return {std::to_string(fromUID), std::move(p->second.event), std::move(p->second.value)};
            }
            throw fflerror("can't find session UID = %llu", to_llu(uid));
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
        R"###( function sendQuery(uid, query)                                                                                                )###""\n"
        R"###(     sendSessionQuery(getSessionUID(), uid, query)                                                                             )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function waitEvent()                                                                                                          )###""\n"
        R"###(     local sessionUID = getSessionUID()                                                                                        )###""\n"
        R"###(     while true do                                                                                                             )###""\n"
        R"###(         local uid, event, value = pollSessionEvent(sessionUID)                                                                )###""\n"
        R"###(         if uid then                                                                                                           )###""\n"
        R"###(             return uid, event, value                                                                                          )###""\n"
        R"###(         end                                                                                                                   )###""\n"
        R"###(         coroutine.yield()                                                                                                     )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function uidQuery(uid, query)                                                                                                 )###""\n"
        R"###(     sendQuery(uid, query)                                                                                                     )###""\n"
        R"###(     local from, event, value = waitEvent()                                                                                    )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###(     if from ~= uid then                                                                                                       )###""\n"
        R"###(         error('Send query to uid ' .. uid .. ' but get response from ' .. from)                                               )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###(     if event ~= SYS_NPCQUERY then                                                                                             )###""\n"
        R"###(         error('Wait event as SYS_NPCQUERY but get ' .. tostring(event))                                                       )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###(     return value                                                                                                              )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function uidQueryName(uid)                                                                                                    )###""\n"
        R"###(     return uidQuery(uid, 'NAME')                                                                                              )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function uidQueryLevel(uid)                                                                                                   )###""\n"
        R"###(     return tonumber(uidQuery(uid, 'LEVEL'))                                                                                   )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function uidQueryGold(uid)                                                                                                    )###""\n"
        R"###(     return tonumber(uidQuery(uid, 'GOLD'))                                                                                    )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function uidPostSell(uid, sell)                                                                                               )###""\n"
        R"###(     if type(sell) ~= 'table' then                                                                                             )###""\n"
        R"###(         error('uidPostSell() expectes a table but get ' .. type(sell))                                                        )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###(     local retVarName = 'ret_varName_' .. getSessionUID()                                                                      )###""\n"
        R"###(     _G[retVarName] = sell                                                                                                     )###""\n"
        R"###(     sendSell(uid, retVarName)                                                                                                 )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( -- setup session table for thread-based parameters                                                                            )###""\n"
        R"###( -- we spawn session by sol::thread which still access global table                                                            )###""\n"
        R"###( -- so we can't have tls per session, have to save session related globals into this table                                     )###""\n"
        R"###( g_sessionTableList = {}                                                                                                       )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function getSessionTable()                                                                                                    )###""\n"
        R"###(     local id, main_thread = coroutine.running()                                                                               )###""\n"
        R"###(     if main_thread then                                                                                                       )###""\n"
        R"###(         error('calling getSessionTable() in main thead')                                                                      )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###(     if not g_sessionTableList[id] then                                                                                        )###""\n"
        R"###(         g_sessionTableList[id] = {}                                                                                           )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###(     return g_sessionTableList[id]                                                                                             )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function clearSessionTable()                                                                                                  )###""\n"
        R"###(     local id, main_thread = coroutine.running()                                                                               )###""\n"
        R"###(     if main_thread then                                                                                                       )###""\n"
        R"###(         error('calling clearSessionTable() in main thead')                                                                    )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###(     g_sessionTableList[id] = nil                                                                                              )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function setSessionUID(uid)                                                                                                   )###""\n"
        R"###(     if not uid then                                                                                                           )###""\n"
        R"###(         error("invalid session uid: nil")                                                                                     )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###(     local sessionTable = getSessionTable()                                                                                    )###""\n"
        R"###(     if sessionTable['SESSION_UID'] then                                                                                       )###""\n"
        R"###(         error('calling setSessionUID() in same thread twice')                                                                 )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###(     sessionTable['SESSION_UID'] = uid                                                                                         )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function getSessionUID()                                                                                                      )###""\n"
        R"###(     local sessionTable = getSessionTable()                                                                                    )###""\n"
        R"###(     if not sessionTable['SESSION_UID'] then                                                                                   )###""\n"
        R"###(         error('session has no uid setup, missed to call setSessionUID(uid) in main()')                                        )###""\n"
        R"###(     end                                                                                                                       )###""\n"
        R"###(     return sessionTable['SESSION_UID']                                                                                        )###""\n"
        R"###( end                                                                                                                           )###""\n"
        R"###(                                                                                                                               )###""\n"
        R"###( function main(uid)                                                                                                            )###""\n"
        R"###(     setSessionUID(uid)                                                                                                        )###""\n"
        R"###(     while true do                                                                                                             )###""\n"
        R"###(         local from, event, value = waitEvent()                                                                                )###""\n"
        R"###(         if has_processNPCEvent(false, event) then                                                                             )###""\n"
        R"###(             processNPCEvent[event](from, value)                                                                               )###""\n"
        R"###(         elseif event == SYS_NPCDONE then                                                                                      )###""\n"
        R"###(             clearSessionTable()                                                                                               )###""\n"
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

void NPChar::LuaNPCModule::setEvent(uint64_t sessionUID, uint64_t from, std::string event, std::string value)
{
    if(!(sessionUID && from && !event.empty())){
        throw fflerror("invalid argument: sessionUID = %llu, from = %llu, event = %s, value = %s", to_llu(sessionUID), to_llu(from), to_cstr(event), to_cstr(value));
    }

    if(event == SYS_NPCDONE){
        m_sessionList.erase(sessionUID);
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

    auto p = m_sessionList.find(sessionUID);
    if(p == m_sessionList.end()){
        if(event != SYS_NPCINIT){
            throw fflerror("get script event %s while no communication initialized", event.c_str());
        }

        LuaNPCModule::LuaNPCSession session;
        session.uid    = sessionUID;
        session.module = this;

        session.co_handler.runner = sol::thread::create(getLuaState().lua_state());
        session.co_handler.callback = sol::state_view(session.co_handler.runner.state())["main"];

        p = m_sessionList.insert({sessionUID, std::move(session)}).first;

        p->second.from = 0;
        p->second.event.clear();
        p->second.value.clear();

        const auto result = p->second.co_handler.callback(std::to_string(sessionUID));
        fnCheckCOResult(result);
    }

    if(p->second.uid != sessionUID){
        throw fflerror("invalid session: session::uid = %llu, sessionUID = %llu", to_llu(p->second.uid), to_llu(sessionUID));
    }

    if(!p->second.co_handler.callback){
        throw fflerror("lua coroutine is not callable");
    }

    // clear the event
    // call the coroutine to make it stuck at pollEvent()

    p->second.from  = from;
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
    sendNetPackage(uid, SM_NPCSELL, cerealf::serialize(SDNPCSell
    {
        .npcUID = UID(),
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
}

void NPChar::sendQuery(uint64_t sessionUID, uint64_t uid, const std::string &query)
{
    AMNPCQuery amNPCQ;
    std::memset(&amNPCQ, 0, sizeof(amNPCQ));

    if(query.size() >= sizeof(amNPCQ.query)){
        throw fflerror("query name is too long: %s", query.c_str());
    }

    std::strcpy(amNPCQ.query, query.c_str());
    m_actorPod->forward(uid, {MPK_NPCQUERY, amNPCQ}, [sessionUID, uid, this](const MessagePack &mpk)
    {
        if(uid != mpk.from()){
            throw fflerror("query sent to uid %llu but get response from %llu", to_llu(uid), to_llu(mpk.from()));
        }

        switch(mpk.Type()){
            case MPK_NPCEVENT:
                {
                    const auto amNPCE = mpk.conv<AMNPCEvent>();
                    m_luaModulePtr->setEvent(sessionUID, uid, amNPCE.event, amNPCE.value);
                    return;
                }
            default:
                {
                    m_luaModulePtr->close(sessionUID);
                    return;
                }
        }
    });
}

void NPChar::sendXMLLayout(uint64_t uid, std::string xmlString)
{
    sendNetPackage(uid, SM_NPCXMLLAYOUT, cerealf::serialize(SDNPCXMLLayout
    {
        .npcUID = UID(),
        .xmlLayout = std::move(xmlString),
    }, true));
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
        case MPK_QUERYSELLITEM:
            {
                on_MPK_QUERYSELLITEM(mpk);
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
