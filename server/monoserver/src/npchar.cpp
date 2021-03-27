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

NPChar::LuaNPCModule::LuaNPCModule(const SDInitNPChar &initParam)
    : ServerLuaModule()
    , m_npcName(initParam.npcName)
{
    fflassert(DBCOM_MAPRECORD(initParam.mapID));
    fflassert(!initParam.npcName.empty());

    m_luaState.set_function("setNPCLook", [done = false, this](int lookID) mutable
    {
        fflassert(!done);
        fflassert(lookID >= 0);

        m_npcLookID = lookID;
        done = true;
    });

    m_luaState.set_function("setNPCGLoc", [done = false, this](sol::variadic_args args) mutable
    {
        const auto [x, y, dir] = [&args]() -> std::tuple<int, int, int>
        {
            const std::vector<sol::object> objArgs(args.begin(), args.end());
            for(const auto &obj: objArgs){
                fflassert(obj.is<int>());
            }

            switch(objArgs.size()){
                case 2 : return {objArgs[0].as<int>(), objArgs[1].as<int>(), 0};
                case 3 : return {objArgs[0].as<int>(), objArgs[1].as<int>(), objArgs[2].as<int>()};
                default: throw fflerror("invalid argument length: %zu", objArgs.size());
            }
        }();

        fflassert(!done);
        fflassert(x >= 0 && y >= 0 && dir >= 0);

        m_npcGLoc.x   = x;
        m_npcGLoc.y   = y;
        m_npcGLoc.dir = dir;
        done = true;
    });

    // NOTE I didn't understand the different between sol::as_table_t and sol:nested
    m_luaState.set_function("setNPCSell", [this](sol::as_table_t<std::vector<std::string>> itemNameList)
    {
        m_npcSell.clear();
        for(const auto &itemName: itemNameList.source){
            if(const auto itemID = DBCOM_ITEMID(to_u8cstr(itemName))){
                m_npcSell.insert(itemID);
            }
        }
    });

    m_luaState.set_function("addNPCSell", [this](std::string itemName)
    {
        if(const auto itemID = DBCOM_ITEMID(to_u8cstr(itemName))){
            m_npcSell.insert(itemID);
        }
    });

    m_luaState.set_function("clearNPCSell", [this]()
    {
        m_npcSell.clear();
    });

    m_luaState.set_function("getUID", [this]() -> std::string
    {
        fflassert(m_npc);
        return std::to_string(m_npc->rawUID());
    });

    m_luaState.set_function("getUIDString", [](std::string uidString) -> std::string
    {
        return uidf::getUIDString(uidf::toUID(uidString));
    });

    m_luaState.set_function("getItemName", [](int itemID) -> std::string
    {
        return to_cstr(DBCOM_ITEMRECORD(itemID).name);
    });

    m_luaState.set_function("getNPCName", [this]() -> std::string
    {
        return m_npcName;
    });

    m_luaState.set_function("getNPCFullName", [mapID = initParam.mapID, this]() -> std::string
    {
        return std::string(to_cstr(DBCOM_MAPRECORD(mapID).name)) + "." + m_npcName;
    });

    m_luaState.set_function("sendSell", [this](std::string uidString)
    {
        fflassert(m_npc);
        m_npc->sendSell(uidf::toUIDEx(uidString));
    });

    m_luaState.set_function("sendCallStackQuery", [this](std::string callStackUID, std::string uidString, std::string query)
    {
        fflassert(m_npc);
        m_npc->sendQuery(uidf::toUIDEx(callStackUID), uidf::toUIDEx(uidString), query);
    });

    m_luaState.set_function("sayXMLString", [this](std::string uidString, std::string xmlString)
    {
        fflassert(m_npc);
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
            m_npc->sendXMLLayout(uid, std::move(xmlString));
        }

        else{
            addLogString(0, u8"invalid UID: 0");
        }
    });

    m_luaState.set_function("pollCallStackEvent", [this](std::string callStackUID)
    {
        fflassert(m_npc);
        const uint64_t uid = [&callStackUID]() -> uint64_t
        {
            try{
                return std::stoull(callStackUID);
            }
            catch(...){
                //
            }
            return 0;
        }();

        return sol::as_returns([uid, this]() -> std::vector<std::string>
        {
            if(auto p = m_callStackList.find(uid); p != m_callStackList.end()){
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
            throw fflerror("can't find call stack UID = %llu", to_llu(uid));
        }());
    });

    m_luaState.script(INCLUA_BEGIN(char)
#include "npchar.lua"
    INCLUA_END());

    m_luaState.script_file([mapID = initParam.mapID, this]() -> std::string
    {
        const auto scriptPath = []() -> std::string
        {
            if(const auto cfgScriptPath = g_serverConfigureWindow->getScriptPath(); !cfgScriptPath.empty()){
                return cfgScriptPath + "/npc";
            }
            return std::string("script/npc");
        }();

        const auto scriptFileName = str_printf("%s/%s.%s.lua", scriptPath.c_str(), to_cstr(DBCOM_MAPRECORD(mapID).name), to_cstr(m_npcName));
        fflassert(filesys::hasFile(scriptFileName.c_str()));
        return scriptFileName;
    }());

    // source script done
    // confirm in the script follow functions get called:
    //
    //     setNPCGLoc()
    //     setNPCLook()
    //
    fflassert(m_npcLookID >= 0);
    fflassert(m_npcGLoc.x >= 0 && m_npcGLoc.y >= 0);

    // NPC script has no state
    // after source it finishes the script, not yield

    m_luaState.script
    (
        R"###( -- do the first sanity check here                               )###""\n"
        R"###( -- last in main call we also check it but with verbose disabled )###""\n"
        R"###( has_processNPCEvent(true, SYS_NPCINIT)                          )###""\n");
}

void NPChar::LuaNPCModule::setEvent(uint64_t callStackUID, uint64_t from, std::string event, std::string value)
{
    if(!(callStackUID && from && !event.empty())){
        throw fflerror("invalid argument: callStackUID = %llu, from = %llu, event = %s, value = %s", to_llu(callStackUID), to_llu(from), to_cstr(event), to_cstr(value));
    }

    if(event == SYS_NPCDONE){
        m_callStackList.erase(callStackUID);
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
            addLogString(1, to_u8cstr(errLine));
        }
    };

    auto p = m_callStackList.find(callStackUID);
    if(p == m_callStackList.end()){
        p = m_callStackList.insert({callStackUID, LuaNPCModule::LuaCallStack(this)}).first;

        // initial call to make main reaches its event polling point
        // need to assign event to let it advance
        const auto result = p->second.co_callback(std::to_string(callStackUID));
        fnCheckCOResult(result);
    }

    if(!p->second.co_callback){
        throw fflerror("lua coroutine is not callable");
    }

    // clear the event
    // call the coroutine to make it stuck at pollEvent()

    p->second.from  = from;
    p->second.event = std::move(event);
    p->second.value = std::move(value);

    const auto result = p->second.co_callback();
    fnCheckCOResult(result);

    if(!p->second.co_callback){
        // not invocable anymore after the event-driven call
        // the event handling coroutine is done
        //
        // remove the call stack when an event sequence is done, i.e.
        // 1. get event SYS_NPCINIT from player, init main()
        // 2. in processNPCEvent(SYS_NPCINIT), script calls uidQueryName(), this sends QUERY_NAME
        // 3. get event NAME
        // 4. in processNPCEvent(SYS_NPCINIT), script calls uidQueryLevel(), this sends QUERY_LEVEL
        // 5. get event LEVEL
        // 6. send sayXML() to player, done main()
        m_callStackList.erase(p);
    }
}

NPChar::NPChar(ServiceCore *core, ServerMap *serverMap, std::unique_ptr<NPChar::LuaNPCModule> luaModulePtr)
    : CharObject
      {
          core,
          serverMap,
          uidf::buildNPCUID(luaModulePtr->getNPCLookID()),
          luaModulePtr->getNPCGLoc().x,
          luaModulePtr->getNPCGLoc().y,
          luaModulePtr->getNPCGLoc().dir,
      }
    , m_luaModulePtr(std::move(luaModulePtr))
{
    // LuaNPCModule(this) can access ``this"
    // when constructing LuaNPCModule we need to confirm ``this" is ready
    m_luaModulePtr->bindNPC(this);
    if(!m_luaModulePtr->getNPCSell().empty()){
        fillSellItemList();
    }
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

void NPChar::sendSell(uint64_t uid)
{
    sendNetPackage(uid, SM_NPCSELL, cerealf::serialize(SDNPCSell
    {
        .npcUID = UID(),
        .itemList = std::vector<uint32_t>(m_luaModulePtr->getNPCSell().begin(), m_luaModulePtr->getNPCSell().end()),
    }, true));
}

void NPChar::sendQuery(uint64_t callStackUID, uint64_t uid, const std::string &query)
{
    AMNPCQuery amNPCQ;
    std::memset(&amNPCQ, 0, sizeof(amNPCQ));

    const auto seqID = m_luaModulePtr->getCallStackSeqID(callStackUID);
    if(!seqID){
        throw fflerror("calling sendQuery(%llu, %llu, %s) outside of LuaCallStack", to_llu(callStackUID), to_llu(uid), to_cstr(query));
    }

    if(query.size() >= sizeof(amNPCQ.query)){
        throw fflerror("query name is too long: %s", query.c_str());
    }

    std::strcpy(amNPCQ.query, query.c_str());
    m_actorPod->forward(uid, {AM_NPCQUERY, amNPCQ}, [callStackUID, uid, seqID, this](const ActorMsgPack &mpk)
    {
        if(uid != mpk.from()){
            throw fflerror("query sent to uid %llu but get response from %llu", to_llu(uid), to_llu(mpk.from()));
        }

        if(mpk.seqID()){
            throw fflerror("query result expects response");
        }

        if(m_luaModulePtr->getCallStackSeqID(callStackUID) != seqID){
            return;
        }

        switch(mpk.type()){
            case AM_NPCEVENT:
                {
                    const auto amNPCE = mpk.conv<AMNPCEvent>();
                    m_luaModulePtr->setEvent(callStackUID, uid, amNPCE.event, amNPCE.value);
                    return;
                }
            default:
                {
                    m_luaModulePtr->close(callStackUID);
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

void NPChar::operateAM(const ActorMsgPack &mpk)
{
    switch(mpk.type()){
        case AM_OFFLINE:
        case AM_METRONOME:
            {
                break;
            }
        case AM_BUY:
            {
                on_AM_BUY(mpk);
                break;
            }
        case AM_ACTION:
            {
                on_AM_ACTION(mpk);
                break;
            }
        case AM_NPCEVENT:
            {
                on_AM_NPCEVENT(mpk);
                break;
            }
        case AM_NOTIFYNEWCO:
            {
                on_AM_NOTIFYNEWCO(mpk);
                break;
            }
        case AM_QUERYCORECORD:
            {
                on_AM_QUERYCORECORD(mpk);
                break;
            }
        case AM_QUERYLOCATION:
            {
                on_AM_QUERYLOCATION(mpk);
                break;
            }
        case AM_QUERYSELLITEMLIST:
            {
                on_AM_QUERYSELLITEMLIST(mpk);
                break;
            }
        case AM_BADACTORPOD:
            {
                on_AM_BADACTORPOD(mpk);
                break;
            }
        default:
            {
                throw fflerror("unsupported message: %s", mpkName(mpk.type()));
            }
    }
}

std::vector<SDCostItem> NPChar::getCostItemList(const SDItem &) const
{
    SDCostItem cost;
    std::vector<SDCostItem> result;

    cost.itemID = DBCOM_ITEMID(u8"金币");
    cost.count  = 100 + std::rand() % 10;

    result.push_back(cost);
    return result;
}

std::set<uint32_t> NPChar::getDefaultSellItemIDList() const
{
    std::set<uint32_t> itemIDList;
    for(uint32_t itemID = 1; itemID < 1000; ++itemID){
        if(const auto &ir = DBCOM_ITEMRECORD(itemID); ir && (std::u8string_view(ir.name) != u8"未知")){
            itemIDList.insert(itemID);
        }
    }
    return itemIDList;
}

void NPChar::fillSellItemList()
{
    for(const uint32_t itemID: m_luaModulePtr->getNPCSell()){
        const auto &ir = DBCOM_ITEMRECORD(itemID);
        if(!ir){
            throw fflerror("selling invalid item: itemID = %llu", to_llu(itemID));
        }

        auto &itemListRef = m_sellItemList[itemID];
        if(ir.packable()){
            // refresh the price for packable item
            // always use seqID = 0 for packable item in the list

            const auto item = createSellItem(itemID, 0);
            const auto cost = getCostItemList(item);

            itemListRef[0] = NPChar::SellItem
            {
                .item = item,
                .locked = false,
                .costList = cost,
            };

            if(itemListRef.size() != 1){
                throw fflerror("failed to reset packable item");
            }
        }
        else{
            const auto fillSize = std::max<size_t>(itemListRef.size(), 20 + std::rand() % 5);
            while(itemListRef.size() < fillSize){
                const uint32_t seqID = itemListRef.empty() ? 1 : (itemListRef.rbegin()->first + 1);
                const auto item = createSellItem(itemID, seqID);
                const auto cost = getCostItemList(item);
                itemListRef[seqID] = NPChar::SellItem
                {
                    .item = item,
                    .locked = false,
                    .costList = cost,
                };
            }
        }
    }
}

SDItem NPChar::createSellItem(uint32_t itemID, uint32_t seqID) const
{
    return SDItem
    {
        .itemID = itemID,
        .seqID =  seqID,
        .count = 1,
        .duration = 0,
        .extAttrList = {},
    };
}
