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
#include "luaf.hpp"
#include "dbpod.hpp"
#include "mathf.hpp"
#include "npchar.hpp"
#include "colorf.hpp"
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

extern DBPod *g_dbPod;
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

    m_luaState.set_function("getNPCName", [this](sol::variadic_args args) -> std::string
    {
        const auto skip = [&args]() -> bool
        {
            switch(args.size()){
                case 0:
                    {
                        return true;
                    }
                case 1:
                    {
                        const sol::object obj(args[0]);
                        if(obj.is<bool>()){
                            return obj.as<bool>();
                        }
                        else{
                            throw fflerror("invalid argument type");
                        }
                    }
                default:
                    {
                        throw fflerror("invalid argument count: %zu", args.size());
                    }
            }
        }();

        if(skip){
            return m_npcName.substr(0, m_npcName.find('_'));
        }
        else{
            return m_npcName;
        }
    });

    m_luaState.set_function("getNPCFullName", [mapID = initParam.mapID, this]() -> std::string
    {
        return std::string(to_cstr(DBCOM_MAPRECORD(mapID).name)) + "." + m_npcName;
    });

    m_luaState.set_function("getNPCMapName", [this](sol::variadic_args args) -> std::string
    {
        fflassert(m_npc);
        const auto skip = [&args]() -> bool
        {
            switch(args.size()){
                case 0:
                    {
                        return true;
                    }
                case 1:
                    {
                        const sol::object obj(args[0]);
                        if(obj.is<bool>()){
                            return obj.as<bool>();
                        }
                        else{
                            throw fflerror("invalid argument type");
                        }
                    }
                default:
                    {
                        throw fflerror("invalid argument count: %zu", args.size());
                    }
            }
        }();

        const std::string mapName = to_cstr(DBCOM_MAPRECORD(m_npc->mapID()).name);
        if(skip){
            return mapName.substr(0, mapName.find('_'));
        }
        else{
            return mapName;
        }
    });

    m_luaState.set_function("getSubukGuildName", [this]() -> std::string
    {
        fflassert(m_npc);
        return "占领沙巴克行会的名字";
    });

    m_luaState.set_function("addMonster", [this](std::string monsterName)
    {
        fflassert(m_npc);
        const auto monsterID = DBCOM_MONSTERID(to_u8cstr(monsterName));

        fflassert(monsterID);
        m_npc->postAddMonster(monsterID);
    });

    m_luaState.set_function("dbSetGKey", [mapID = initParam.mapID, this](std::string key, sol::object obj)
    {
        fflassert(m_npc);
        const auto npcDBName = str_printf("tbl_global_npcdb_%s_%s", to_cstr(DBCOM_MAPRECORD(mapID).name), m_npcName.c_str());

        if(!g_dbPod->createQuery(u8R"###(select name from sqlite_master where type='table' and name='%s')###", npcDBName.c_str()).executeStep()){
            g_dbPod->exec(
                    u8R"###( create table %s(                         )###"
                    u8R"###(     fld_key   text not null primary key, )###"
                    u8R"###(     fld_value blob not null              )###"
                    u8R"###( );                                       )###", npcDBName.c_str());
        }

        auto query = g_dbPod->createQuery(u8R"###(replace into %s(fld_key, fld_value) values('%s', ?))###", npcDBName.c_str(), key.c_str());
        query.bind(1, luaf::buildBlob(obj));
        query.exec();
    });

    m_luaState.set_function("dbGetGKey", [mapID = initParam.mapID, this](std::string key, sol::this_state s) -> sol::object
    {
        fflassert(m_npc);
        fflassert(!key.empty());
        const auto npcDBName = str_printf("tbl_global_npcdb_%s_%s", to_cstr(DBCOM_MAPRECORD(mapID).name), m_npcName.c_str());

        sol::state_view sv(s);
        if(!g_dbPod->createQuery(u8R"###(select name from sqlite_master where type='table' and name='%s')###", npcDBName.c_str()).executeStep()){
            return sol::make_object(sv, sol::nil);
        }

        auto queryStatement = g_dbPod->createQuery(u8R"###(select fld_value from %s where fld_key='%s')###", npcDBName.c_str(), key.c_str());
        if(!queryStatement.executeStep()){
            return sol::make_object(sv, sol::nil);
        }
        return luaf::buildLuaObj(sv, queryStatement.getColumn(0).getString());
    });

    m_luaState.set_function("uidDBSetKey", [mapID = initParam.mapID, this](std::string uidString, std::string key, sol::object obj)
    {
        fflassert(m_npc);
        const auto dbid = uidf::getPlayerDBID(uidf::toUIDEx(uidString));
        const auto npcDBName = str_printf("tbl_npcdb_%s_%s", to_cstr(DBCOM_MAPRECORD(mapID).name), m_npcName.c_str());

        if(!g_dbPod->createQuery(u8R"###(select name from sqlite_master where type='table' and name='%s')###", npcDBName.c_str()).executeStep()){
            g_dbPod->exec(u8R"###(create table %s(fld_dbid integer not null primary key))###", npcDBName.c_str());
        }

        const auto colType = [&npcDBName, &key]() -> std::string
        {
            auto queryTableInfo= g_dbPod->createQuery(u8R"###(pragma table_info(%s))###", npcDBName.c_str());
            while(queryTableInfo.executeStep()){
                if(queryTableInfo.getColumn("name").getText() == key){
                    return queryTableInfo.getColumn("type").getText();
                }
            }
            return {};
        }();

        const auto objType = [&key, &obj]() -> std::string
        {
            if(obj.is<int>()){
                return "integer";
            }
            else if(obj.is<double>()){
                return "real";
            }
            else if(obj.is<std::string>()){
                return "text";
            }
            else{
                throw fflerror("invalid object type: name = %s", to_cstr(key));
            }
        }();

        if(colType.empty()){
            g_dbPod->exec(u8R"###(alter table %s add column %s %s)###", npcDBName.c_str(), key.c_str(), objType.c_str());
        }
        else if(colType != objType){
            throw fflerror("column type mismatch, expected %s:%s, get type %s", npcDBName.c_str(), key.c_str(), objType.c_str());
        }

        if(objType == "integer"){
            g_dbPod->exec(u8R"###(insert into %s(fld_dbid, %s) values(%llu, %d) on conflict(fld_dbid) do update set %s=%d)###", npcDBName.c_str(), key.c_str(), to_llu(dbid), obj.as<int>(), key.c_str(), obj.as<int>());
        }
        else if(objType == "real"){
            g_dbPod->exec(u8R"###(insert into %s(fld_dbid, %s) values(%llu, %f) on conflict(fld_dbid) do update set %s=%f)###", npcDBName.c_str(), key.c_str(), to_llu(dbid), obj.as<double>(), key.c_str(), obj.as<double>());
        }
        else{
            g_dbPod->exec(u8R"###(insert into %s(fld_dbid, %s) values(%llu, '%s') on conflict(fld_dbid) do update set %s='%s')###", npcDBName.c_str(), key.c_str(), to_llu(dbid), obj.as<std::string>().c_str(), key.c_str(), obj.as<std::string>().c_str());
        }
    });

    m_luaState.set_function("uidDBGetKey", [mapID = initParam.mapID, this](std::string uidString, std::string key, sol::this_state s) -> sol::object
    {
        fflassert(m_npc);
        fflassert(!key.empty());

        const auto dbid = uidf::getPlayerDBID(uidf::toUIDEx(uidString));
        const auto npcDBName = str_printf("tbl_npcdb_%s_%s", to_cstr(DBCOM_MAPRECORD(mapID).name), m_npcName.c_str());

        sol::state_view sv(s);
        if(!g_dbPod->createQuery(u8R"###(select name from sqlite_master where type='table' and name='%s')###", npcDBName.c_str()).executeStep()){
            return sol::make_object(sv, sol::nil);
        }

        auto queryStatement = g_dbPod->createQuery(u8R"###(select %s from %s where fld_dbid=%llu)###", key.c_str(), npcDBName.c_str(), to_llu(dbid));
        if(!queryStatement.executeStep()){
            return sol::make_object(sv, sol::nil);
        }

        switch(const auto column = queryStatement.getColumn(0); column.getType()){
            case SQLITE_INTEGER:
                {
                    return sol::object(sv, sol::in_place_type<int>, column.getInt());
                }
            case SQLITE_FLOAT:
                {
                    return sol::object(sv, sol::in_place_type<double>, column.getDouble());
                }
            case SQLITE_TEXT:
                {
                    return sol::object(sv, sol::in_place_type<std::string>, column.getText());
                }
            case SQLITE_NULL:
                {
                    return sol::make_object(sv, sol::nil);
                }
            default:
                {
                    throw fflerror("column type not supported: %d", column.getType());
                }
        }
    });

    m_luaState.set_function("uidPostSell", [this](std::string uidString)
    {
        fflassert(m_npc);
        m_npc->postSell(uidf::toUIDEx(uidString));
    });

    m_luaState.set_function("uidPostStartInvOp", [this](std::string uidString, int invOp, std::string queryTag, std::string commitTag, sol::as_table_t<std::vector<std::string>> typeTable)
    {
        fflassert(m_npc);
        fflassert(invOp >= INVOP_BEGIN);
        fflassert(invOp <  INVOP_END);

        std::set<std::u8string> typeList;
        for(const auto &type: typeTable.source){
            typeList.insert(to_u8cstr(type));
        }
        m_npc->postStartInvOp(uidf::toUIDEx(uidString), invOp, queryTag, commitTag, {typeList.begin(), typeList.end()});
    });

    m_luaState.set_function("uidPostInvOpCost", [this](std::string uidString, int invOp, int itemID, int seqID, int cost)
    {
        fflassert(m_npc);
        fflassert(invOp >= INVOP_BEGIN);
        fflassert(invOp <  INVOP_END);

        m_npc->postInvOpCost(uidf::toUIDEx(uidString), invOp, itemID, seqID, cost);
    });

    m_luaState.set_function("uidPostStartInput", [this](std::string uidString, std::string title, std::string commitTag, bool show)
    {
        fflassert(m_npc);
        fflassert(!title.empty());
        fflassert(!commitTag.empty());
        m_npc->postStartInput(uidf::toUIDEx(uidString), title, commitTag, show);
    });

    m_luaState.set_function("uidPostXMLString", [this](std::string uidString, std::string xmlString)
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
            m_npc->postXMLLayout(uid, std::move(xmlString));
        }

        else{
            addLogString(0, u8"invalid UID: 0");
        }
    });

    m_luaState.set_function("sendCallStackQuery", [this](std::string callStackUID, std::string uidString, std::string query)
    {
        fflassert(m_npc);
        m_npc->sendQuery(uidf::toUIDEx(callStackUID), uidf::toUIDEx(uidString), query);
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


                if(!p->second.value.has_value()){
                    return {std::to_string(fromUID), std::move(p->second.event)};
                }
                return {std::to_string(fromUID), std::move(p->second.event), std::move(p->second.value.value())};
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
            if(const auto cfgScriptPath = g_serverConfigureWindow->getConfig().scriptPath; !cfgScriptPath.empty()){
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
    fflassert(m_npcGLoc.x >= 0 && m_npcGLoc.y >= 0 && m_npcGLoc.dir >= 0);

    // NPC script has no state
    // after source it finishes the script, not yield

    m_luaState.script
    (
        R"###( -- do the first sanity check here                               )###""\n"
        R"###( -- last in main call we also check it but with verbose disabled )###""\n"
        R"###( has_processNPCEvent(true, SYS_NPCINIT)                          )###""\n");
}

void NPChar::LuaNPCModule::setEvent(uint64_t callStackUID, uint64_t from, std::string event, std::optional<std::string> value)
{
    if(!(callStackUID && from && !event.empty())){
        throw fflerror("invalid argument: callStackUID = %llu, from = %llu, event = %s, value = %s", to_llu(callStackUID), to_llu(from), to_cstr(event), to_cstr(value.value_or("(nil)")));
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
        // 6. send uidPostXML() to player, done main()
        m_callStackList.erase(p);
    }
}

NPChar::NPChar(const ServerMap *mapCPtr, std::unique_ptr<NPChar::LuaNPCModule> luaModulePtr)
    : CharObject
      {
          mapCPtr,
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

void NPChar::reportCO(uint64_t)
{
}
bool NPChar::goDie()
{
    return true;
}

bool NPChar::goGhost()
{
    return true;
}

void NPChar::postSell(uint64_t uid)
{
    forwardNetPackage(uid, SM_NPCSELL, cerealf::serialize(SDNPCSell
    {
        .npcUID = UID(),
        .itemList = std::vector<uint32_t>(m_luaModulePtr->getNPCSell().begin(), m_luaModulePtr->getNPCSell().end()),
    }));
}

void NPChar::postInvOpCost(uint64_t uid, int invOp, uint32_t itemID, uint32_t seqID, size_t cost)
{
    fflassert(invOp >= INVOP_BEGIN);
    fflassert(invOp <  INVOP_END);

    SMInvOpCost smIOPC;
    std::memset(&smIOPC, 0, sizeof(smIOPC));

    smIOPC.invOp = invOp;
    smIOPC.itemID = itemID;
    smIOPC.seqID = seqID;
    smIOPC.cost = to_u32(cost);

    forwardNetPackage(uid, SM_INVOPCOST, smIOPC);
}

void NPChar::postStartInvOp(uint64_t uid, int invOp, std::string queryTag, std::string commitTag, std::vector<std::u8string> typeList)
{
    fflassert(invOp >= INVOP_BEGIN);
    fflassert(invOp <  INVOP_END);

    forwardNetPackage(uid, SM_STARTINVOP, cerealf::serialize(SDStartInvOp
    {
        .invOp = invOp,
        .uid= UID(),
        .queryTag = queryTag,
        .commitTag = commitTag,
        .typeList = typeList,
    }));
}

void NPChar::postStartInput(uint64_t uid, std::string title, std::string commitTag, bool show)
{
    forwardNetPackage(uid, SM_STARTINPUT, cerealf::serialize(SDStartInput
    {
        .uid = UID(),
        .title = title,
        .commitTag = commitTag,
        .show = show,
    }));
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
    m_actorPod->forward(uid, {AM_NPCQUERY, amNPCQ}, [callStackUID, uid, seqID, query /* not ref */, this](const ActorMsgPack &mpk)
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
                    const auto sdNPCE = mpk.deserialize<SDNPCEvent>();
                    if(!query.starts_with(sdNPCE.event)){
                        throw fflerror("invalid response: query = %s, event = %s", to_cstr(query), to_cstr(sdNPCE.event));
                    }

                    m_luaModulePtr->setEvent(callStackUID, uid, SYS_NPCQUERY, sdNPCE.value);
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

void NPChar::postXMLLayout(uint64_t uid, std::string xmlString)
{
    forwardNetPackage(uid, SM_NPCXMLLAYOUT, cerealf::serialize(SDNPCXMLLayout
    {
        .npcUID = UID(),
        .xmlLayout = std::move(xmlString),
    }));
}

void NPChar::postAddMonster(uint32_t monsterID)
{
    fflassert(monsterID);

    AMAddCharObject amACO;
    std::memset(&amACO, 0, sizeof(amACO));

    amACO.type = UID_MON;
    amACO.x = X();
    amACO.y = Y() + 1;
    amACO.mapID = mapID();
    amACO.strictLoc = false;

    amACO.monster.monsterID = monsterID;
    amACO.monster.masterUID = 0;

    m_actorPod->forward(m_map->UID(), {AM_ADDCO, amACO}, [](const ActorMsgPack &rmpk)
    {
        switch(rmpk.type()){
            case AM_UID:
                {
                    if(const auto amUID = rmpk.conv<AMUID>(); amUID.UID){
                        return;
                    }
                    break;
                }
            default:
                {
                    break;
                }
        }
        g_monoServer->addLog(LOGTYPE_WARNING, "NPC failed to add monster");
    });
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
        case AM_ATTACK:
            {
                on_AM_ATTACK(mpk);
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
    return
    {
        SDCostItem
        {
            .itemID = DBCOM_ITEMID(u8"金币（小）"),
            .count  = to_uz(mathf::rand(90, 110)),
        },
    };
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
    const auto &ir = DBCOM_ITEMRECORD(itemID);
    fflassert(ir);

    if(ir.isDress()){
        return SDItem
        {
            .itemID = itemID,
            .seqID = seqID,
            .count = 1,
            .duration = {mathf::rand<size_t>(0, ir.equip.duration), to_uz(ir.equip.duration)},
            .extAttrList
            {
                {SDItem::EA_COLOR, colorf::RGBA(mathf::rand(100, 255), mathf::rand(100, 255), mathf::rand(100, 255), 0XFF)},
            },
        };
    }
    else if(ir.isWeapon()){
        return SDItem
        {
            .itemID = itemID,
            .seqID = seqID,
            .count = 1,
            .duration = {mathf::rand<size_t>(0, ir.equip.duration), to_uz(ir.equip.duration)},
            .extAttrList
            {
                {SDItem::EA_DC, mathf::rand<int>(1, 5)},
                {SDItem::EA_MC, mathf::rand<int>(1, 5)},
                {SDItem::EA_SC, mathf::rand<int>(1, 5)},

                // {SDItem::EA_DCHIT, mathf::rand<int>(1, 5)},
                // {SDItem::EA_MCHIT, mathf::rand<int>(1, 5)},
                // {SDItem::EA_DCDODGE, mathf::rand<int>(1, 5)},
                // {SDItem::EA_MCDODGE, mathf::rand<int>(1, 5)},
                //
                // {SDItem::EA_SPEED, mathf::rand<int>(1, 5)},
                // {SDItem::EA_COMFORT, mathf::rand<int>(1, 5)},
                // {SDItem::EA_LUCKCURSE, mathf::rand<int>(1, 5)},
                //
                // {SDItem::EA_HPADD, mathf::rand<int>(1, 5)},
                // {SDItem::EA_HPSTEAL, mathf::rand<int>(1, 5)},
                // {SDItem::EA_HPRECOVER, mathf::rand<int>(1, 5)},
                //
                // {SDItem::EA_MPADD, mathf::rand<int>(1, 5)},
                // {SDItem::EA_MPSTEAL, mathf::rand<int>(1, 5)},
                // {SDItem::EA_MPRECOVER, mathf::rand<int>(1, 5)},
                //
                // {SDItem::EA_DCFIRE, mathf::rand<int>(1, 5)},
                // {SDItem::EA_DCICE, mathf::rand<int>(1, 5)},
                // {SDItem::EA_DCLIGHT, mathf::rand<int>(1, 5)},
                // {SDItem::EA_DCWIND, mathf::rand<int>(1, 5)},
                // {SDItem::EA_DCHOLY, mathf::rand<int>(1, 5)},
                // {SDItem::EA_DCDARK, mathf::rand<int>(1, 5)},
                // {SDItem::EA_DCPHANTOM, mathf::rand<int>(1, 5)},
                //
                // {SDItem::EA_ACFIRE, mathf::rand<int>(1, 5)},
                // {SDItem::EA_ACICE, mathf::rand<int>(1, 5)},
                // {SDItem::EA_ACLIGHT, mathf::rand<int>(1, 5)},
                // {SDItem::EA_ACWIND, mathf::rand<int>(1, 5)},
                // {SDItem::EA_ACHOLY, mathf::rand<int>(1, 5)},
                // {SDItem::EA_ACDARK, mathf::rand<int>(1, 5)},
                // {SDItem::EA_ACPHANTOM, mathf::rand<int>(1, 5)},

                {SDItem::EA_BUFFID, [itemID]() -> uint32_t
                {
                    switch(itemID){
                        case DBCOM_ITEMID(u8"龙纹剑"): return DBCOM_BUFFID(u8"龙纹圣光");
                        default                      : return DBCOM_BUFFID(u8"死亡威慑");
                    }
                }()},
            },
        };
    }
    else if(ir.isRing()){
        return SDItem
        {
            .itemID = itemID,
            .seqID = seqID,
            .count = 1,
            .duration = {mathf::rand<size_t>(0, ir.equip.duration), to_uz(ir.equip.duration)},
            .extAttrList
            {
                {SDItem::EA_DC, mathf::rand<int>(1, 5)},
                {SDItem::EA_BUFFID, DBCOM_BUFFID(u8"吸血鬼的诅咒")},
            },
        };
    }
    else if(ir.isHelmet()){
        return SDItem
        {
            .itemID = itemID,
            .seqID = seqID,
            .count = 1,
            .duration = {mathf::rand<size_t>(0, ir.equip.duration), to_uz(ir.equip.duration)},
            .extAttrList
            {
                {SDItem::EA_AC, mathf::rand<int>(1, 5)},
                {SDItem::EA_MAC, mathf::rand<int>(1, 5)},
            },
        };
    }
    else{
        return SDItem
        {
            .itemID = itemID,
            .seqID = seqID,
            .count = 1,
            .extAttrList = {},
        };
    }
}
