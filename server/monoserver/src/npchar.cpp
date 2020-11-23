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
#include "totype.hpp"
#include "uidf.hpp"
#include "npchar.hpp"
#include "fflerror.hpp"
#include "friendtype.hpp"
#include "monoserver.hpp"
#include "serverconfigurewindow.hpp"

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
        if(!npc->m_npcName.empty()){
            return npc->m_npcName;
        }

        if(npc->rawUID()){
            return uidf::getUIDString(npc->rawUID());
        }
        return std::string("ZERO");
    });

    m_luaState.set_function("setNPCName", [npc](std::string npcName)
    {
        npc->m_npcName = npcName;
    });

    m_luaState.set_function("sendQuery", [npc](std::string uidString, std::string queryName)
    {
        npc->sendQuery(uidf::toUIDEx(uidString), queryName);
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
        R"###( function queryLevel(uid)                                                     )###""\n"
        R"###(     sendQuery(uid, 'LEVEL')                                                  )###""\n"
        R"###(     local event, value = waitEvent(uid)                                      )###""\n"
        R"###(     if event ~= SYS_NPCQUERY then                                            )###""\n"
        R"###(         addLog(1, 'Wait event as SYS_NPCQUERY but get ' .. tostring(event))  )###""\n"
        R"###(         return nil                                                           )###""\n"
        R"###(     else                                                                     )###""\n"
        R"###(         return tonumber(value)                                               )###""\n"
        R"###(     end                                                                      )###""\n"
        R"###( end                                                                          )###""\n"
        R"###(                                                                              )###""\n"
        R"###( function queryGold(uid)                                                      )###""\n"
        R"###(     sendQuery(uid, 'GOLD')                                                   )###""\n"
        R"###(     local event, value = waitEvent(uid)                                      )###""\n"
        R"###(     if event ~= SYS_NPCQUERY then                                            )###""\n"
        R"###(         addLog(1, 'Wait event as SYS_NPCQUERY but get ' .. tostring(event))  )###""\n"
        R"###(         return nil                                                           )###""\n"
        R"###(     else                                                                     )###""\n"
        R"###(         return tonumber(value)                                               )###""\n"
        R"###(     end                                                                      )###""\n"
        R"###( end                                                                          )###""\n"
        R"###(                                                                              )###""\n"
        R"###( function waitEvent(uid)                                                      )###""\n"
        R"###(     while true do                                                            )###""\n"
        R"###(         local event, value = pollEvent(uid)                                  )###""\n"
        R"###(         if event then                                                        )###""\n"
        R"###(             return event, value                                              )###""\n"
        R"###(         end                                                                  )###""\n"
        R"###(         coroutine.yield()                                                    )###""\n"
        R"###(     end                                                                      )###""\n"
        R"###( end                                                                          )###""\n"
        R"###(                                                                              )###""\n"
        R"###( function main(uid)                                                           )###""\n"
        R"###(     while true do                                                            )###""\n"
        R"###(         local event, value = waitEvent(uid)                                  )###""\n"
        R"###(         if type(processNPCEvent[event]) == 'function' then                   )###""\n"
        R"###(             processNPCEvent[event](uid, value)                               )###""\n"
        R"###(         end                                                                  )###""\n"
        R"###(     end                                                                      )###""\n"
        R"###( end                                                                          )###""\n"
    );

    m_luaState.script_file([]() -> std::string
    {
        if(const auto scriptPath = g_serverConfigureWindow->getScriptPath(); !scriptPath.empty()){
            return scriptPath + "npc/default.lua";
        }
        return std::string("script/npc/default.lua");
    }());
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

        session.event.clear();
        session.value.clear();
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

NPChar::NPChar(uint16_t lookId, ServiceCore *core, ServerMap *serverMap, int mapX, int mapY, int dirIndex)
    : CharObject(core, serverMap, uidf::buildNPCUID(lookId), mapX, mapY, DIR_NONE)
    , m_dirIndex(dirIndex)
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

void NPChar::sendQuery(uint64_t uid, const std::string &queryName)
{
    AMNPCQuery amNPCQ;
    std::memset(&amNPCQ, 0, sizeof(amNPCQ));

    if(queryName.size() >= sizeof(amNPCQ.query)){
        throw fflerror("query name is too long: %s", queryName.c_str());
    }

    std::strcpy(amNPCQ.query, queryName.c_str());
    m_actorPod->forward(uid, {MPK_NPCQUERY, amNPCQ}, [uid, this](const MessagePack &mpk)
    {
        switch(mpk.Type()){
            case MPK_NPCEVENT:
                {
                    const auto amNPCE = mpk.conv<AMNPCEvent>();
                    m_luaModulePtr->setEvent(mpk.from(), amNPCE.event, amNPCE.value);
                    return;
                }
            default:
                {
                    m_luaModulePtr->close(uid);
                    return;
                }
        }
    });
}

void NPChar::sendXMLLayout(uint64_t uid, const char *xmlString)
{
    if(!xmlString){
        throw fflerror("xmlString null");
    }

    AMNPCXMLLayout amNPCXMLL;
    std::memset(&amNPCXMLL, 0, sizeof(amNPCXMLL));

    if(std::strlen(xmlString) > sizeof(amNPCXMLL.xmlLayout)){
        throw fflerror("xmlString is too long");
    }

    std::strcpy(amNPCXMLL.xmlLayout, xmlString);
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
                throw fflerror("unsupported message: %s", mpk.Name());
            }
    }
}
