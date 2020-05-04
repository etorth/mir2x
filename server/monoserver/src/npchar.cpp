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
#include "fflerror.hpp"
#include "monoserver.hpp"
#include "serverconfigurewindow.hpp"

extern ServerConfigureWindow *g_ServerConfigureWindow;

NPChar::LuaNPCModule::LuaNPCModule(NPChar *npc)
    : ServerLuaModule()
    , m_NPChar(npc)
{
    m_LuaState.set_function("getUID", [this]() -> std::string
    {
        return std::to_string(m_NPChar->rawUID());
    });

    m_LuaState.set_function("getUIDString", [](std::string uidString) -> std::string
    {
        const uint64_t uid = [&uidString]() -> uint64_t
        {
            try{
                return std::stoull(uidString);
            }
            catch(...){
                return 0;
            }
        }();
        return uidf::getUIDString(uid);
    });

    m_LuaState.set_function("getName", [this]() -> std::string
    {
        if(m_NPChar->rawUID()){
            return uidf::getUIDString(m_NPChar->rawUID());
        }
        return std::string("ZERO");
    });

    m_LuaState.set_function("sayXML", [this](std::string uidString, std::string xmlString)
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
            m_NPChar->sendXMLLayout(uid, xmlString.c_str());
        }

        else{
            addLog(0, "invalid UID: 0");
        }
    });

    m_LuaState.set_function("pollEvent", [this](std::string uidString)
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
            throw fflerror("can't find session for UID = %llu", toLLU(uid));
        }());
    });

    m_LuaState.script
    (
        R"###( function waitEvent(uid)                                        )###""\n"
        R"###(     while true do                                              )###""\n"
        R"###(         local event, value = pollEvent(uid)                    )###""\n"
        R"###(         if event then                                          )###""\n"
        R"###(             return event, value                                )###""\n"
        R"###(         end                                                    )###""\n"
        R"###(         coroutine.yield()                                      )###""\n"
        R"###(     end                                                        )###""\n"
        R"###( end                                                            )###""\n"
        R"###(                                                                )###""\n"
        R"###( function main(uid)                                             )###""\n"
        R"###(     while true do                                              )###""\n"
        R"###(         local event, value = waitEvent(uid)                    )###""\n"
        R"###(         if type(processNPCEvent[event]) == 'function' then     )###""\n"
        R"###(             processNPCEvent[event](uid, value)                 )###""\n"
        R"###(         end                                                    )###""\n"
        R"###(     end                                                        )###""\n"
        R"###( end                                                            )###""\n"
    );

    m_LuaState.script_file([]() -> std::string
    {
        if(const auto scriptPath = g_ServerConfigureWindow->GetScriptPath(); !scriptPath.empty()){
            return scriptPath + "npc/default.lua";
        }
        return std::string("script/npc/default.lua");
    }());
}

void NPChar::LuaNPCModule::setEvent(uint64_t uid, std::string event, std::string value)
{
    if(!(uid && !event.empty())){
        throw fflerror("invalid argument: uid = %llu, event = %s, value = %s", toLLU(uid), event.c_str(), value.c_str());
    }

    auto p = m_sessionList.find(uid);
    if(p == m_sessionList.end()){
        if(event != "npc_init"){
            throw fflerror("get script event %s while no communication initialized", event.c_str());
        }

        LuaNPCModule::LuaNPCSession session
        {
            uid,
            {},
            {},
            this,
            m_LuaState["main"],
        };

        // create the coroutine
        // and make it get stuck at pollEvent()

        p = m_sessionList.insert({uid, std::move(session)}).first;
        p->second.co_handler(std::to_string(uid));
    }

    if(p->second.uid != uid){
        throw fflerror("invalid session: key = %llu, value.key = %llu", toLLU(uid), toLLU(p->second.uid));
    }

    if(!p->second.co_handler){
        throw fflerror("lua coroutine is not callable");
    }

    p->second.event = event;
    p->second.value = value;
    p->second.co_handler();
}

NPChar::NPChar(uint16_t lookId, ServiceCore *core, ServerMap *serverMap, int mapX, int mapY, int dirIndex)
    : CharObject(core, serverMap, uidf::buildNPCUID(lookId), mapX, mapY, DIR_NONE)
    , m_dirIndex(dirIndex)
    , m_luaModule(this)
{}

bool NPChar::Update()
{
    return true;
}

bool NPChar::InRange(int, int, int)
{
    return true;
}

void NPChar::ReportCORecord(uint64_t)
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

void NPChar::checkFriend(uint64_t, std::function<void(int)>)
{
}

bool NPChar::GoDie()
{
    return true;
}

bool NPChar::GoGhost()
{
    return true;
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

void NPChar::OperateAM(const MessagePack &mpk)
{
    switch(mpk.Type()){
        case MPK_OFFLINE:
        case MPK_METRONOME:
            {
                break;
            }
        case MPK_ACTION:
            {
                On_MPK_ACTION(mpk);
                break;
            }
        case MPK_NPCEVENT:
            {
                On_MPK_NPCEVENT(mpk);
                break;
            }
        case MPK_NOTIFYNEWCO:
            {
                On_MPK_NOTIFYNEWCO(mpk);
                break;
            }
        case MPK_QUERYCORECORD:
            {
                On_MPK_QUERYCORECORD(mpk);
                break;
            }
        case MPK_QUERYLOCATION:
            {
                On_MPK_QUERYLOCATION(mpk);
                break;
            }
        default:
            {
                throw fflerror("unsupported message: %s", mpk.Name());
            }
    }
}
