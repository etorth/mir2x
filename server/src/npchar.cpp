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
#include "serverconfigurewindow.hpp"

extern DBPod *g_dbPod;
extern MonoServer *g_monoServer;
extern ServerConfigureWindow *g_serverConfigureWindow;

NPChar::LuaNPCModule::LuaNPCModule(NPChar *npcPtr, const std::string &scriptName)
    : ServerLuaModule()
    , m_npc(npcPtr)
{
    fflassert(npcPtr);
    fflassert(str_haschar(scriptName));

    // NOTE I didn't understand the different between sol::as_table_t and sol:nested
    bindFunction("setNPCSell", [this](sol::as_table_t<std::vector<std::string>> itemNameList)
    {
        m_npcSell.clear();
        for(const auto &itemName: itemNameList.value()){
            if(const auto itemID = DBCOM_ITEMID(to_u8cstr(itemName))){
                m_npcSell.insert(itemID);
            }
        }
    });

    bindFunction("addNPCSell", [this](std::string itemName)
    {
        if(const auto itemID = DBCOM_ITEMID(to_u8cstr(itemName))){
            m_npcSell.insert(itemID);
        }
    });

    bindFunction("clearNPCSell", [this]()
    {
        m_npcSell.clear();
    });

    bindFunction("getUID", [this]() -> std::string
    {
        return std::to_string(m_npc->rawUID());
    });

    bindFunction("getUIDString", [](uint64_t uid) -> std::string
    {
        return uidf::getUIDString(uid);
    });

    bindFunction("getNPCName", [this](sol::variadic_args args) -> std::string
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
            return m_npc->getNPCName().substr(0, m_npc->getNPCName().find('_'));
        }
        else{
            return m_npc->getNPCName();
        }
    });

    bindFunction("getNPCFullName", [this]() -> std::string
    {
        return std::string(to_cstr(DBCOM_MAPRECORD(m_npc->mapID()).name)) + "." + m_npc->getNPCName();
    });

    bindFunction("getNPCMapName", [this](sol::variadic_args args) -> std::string
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

        const std::string mapName = to_cstr(DBCOM_MAPRECORD(m_npc->mapID()).name);
        if(skip){
            return mapName.substr(0, mapName.find('_'));
        }
        else{
            return mapName;
        }
    });

    bindFunction("getSubukGuildName", [this]() -> std::string
    {
        return "占领沙巴克行会的名字";
    });

    bindFunction("addMonster", [this](std::string monsterName)
    {
        const auto monsterID = DBCOM_MONSTERID(to_u8cstr(monsterName));

        fflassert(monsterID);
        m_npc->postAddMonster(monsterID);
    });

    bindFunction("dbSetGKey", [this](std::string key, sol::object obj)
    {
        const auto npcDBName = str_printf("tbl_global_npcdb_%s_%s", to_cstr(DBCOM_MAPRECORD(m_npc->mapID()).name), m_npc->getNPCName().c_str());
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

    bindFunction("dbGetGKey", [this](std::string key, sol::this_state s) -> sol::object
    {
        fflassert(!key.empty());
        const auto npcDBName = str_printf("tbl_global_npcdb_%s_%s", to_cstr(DBCOM_MAPRECORD(m_npc->mapID()).name), m_npc->getNPCName().c_str());

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

    bindFunction("uidDBSetKey", [this](uint64_t uid, std::string key, sol::object obj)
    {
        const auto dbid = uidf::getPlayerDBID(uid);
        const auto npcDBName = str_printf("tbl_npcdb_%s_%s", to_cstr(DBCOM_MAPRECORD(m_npc->mapID()).name), m_npc->getNPCName().c_str());

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
                return "INTEGER";
            }
            else if(obj.is<double>()){
                return "REAL";
            }
            else if(obj.is<std::string>()){
                return "TEXT";
            }
            else{
                throw fflerror("invalid object type: name = %s", to_cstr(key));
            }
        }();

        if(colType.empty()){
            g_dbPod->exec(u8R"###(alter table %s add column %s %s)###", npcDBName.c_str(), key.c_str(), objType.c_str());
        }
        else if(colType != objType){
            throw fflerror("column %s:%s type mismatch, expected %s, get type %s", npcDBName.c_str(), key.c_str(), colType.c_str(), objType.c_str());
        }

        if(objType == "INTEGER"){
            g_dbPod->exec(u8R"###(insert into %s(fld_dbid, %s) values(%llu, %d) on conflict(fld_dbid) do update set %s=%d)###", npcDBName.c_str(), key.c_str(), to_llu(dbid), obj.as<int>(), key.c_str(), obj.as<int>());
        }
        else if(objType == "REAL"){
            g_dbPod->exec(u8R"###(insert into %s(fld_dbid, %s) values(%llu, %f) on conflict(fld_dbid) do update set %s=%f)###", npcDBName.c_str(), key.c_str(), to_llu(dbid), obj.as<double>(), key.c_str(), obj.as<double>());
        }
        else{
            g_dbPod->exec(u8R"###(insert into %s(fld_dbid, %s) values(%llu, '%s') on conflict(fld_dbid) do update set %s='%s')###", npcDBName.c_str(), key.c_str(), to_llu(dbid), obj.as<std::string>().c_str(), key.c_str(), obj.as<std::string>().c_str());
        }
    });

    bindFunction("uidDBGetKey", [this](uint64_t uid, std::string key, sol::this_state s) -> sol::object
    {
        fflassert(!key.empty());
        const auto dbid = uidf::getPlayerDBID(uid);
        const auto npcDBName = str_printf("tbl_npcdb_%s_%s", to_cstr(DBCOM_MAPRECORD(m_npc->mapID()).name), m_npc->getNPCName().c_str());

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

    bindFunction("uidPostSell", [this](uint64_t uid)
    {
        m_npc->postSell(uid);
    });

    bindFunction("uidPostStartInvOp", [this](uint64_t uid, int invOp, std::string queryTag, std::string commitTag, sol::as_table_t<std::vector<std::string>> typeTable)
    {
        fflassert(invOp >= INVOP_BEGIN);
        fflassert(invOp <  INVOP_END);

        std::set<std::u8string> typeList;
        for(const auto &type: typeTable.value()){
            typeList.insert(to_u8cstr(type));
        }
        m_npc->postStartInvOp(uid, invOp, queryTag, commitTag, {typeList.begin(), typeList.end()});
    });

    bindFunction("uidPostInvOpCost", [this](uint64_t uid, int invOp, int itemID, int seqID, int cost)
    {
        fflassert(invOp >= INVOP_BEGIN);
        fflassert(invOp <  INVOP_END);

        m_npc->postInvOpCost(uid, invOp, itemID, seqID, cost);
    });

    bindFunction("uidPostStartInput", [this](uint64_t uid, std::string title, std::string commitTag, bool show)
    {
        fflassert(!title.empty());
        fflassert(!commitTag.empty());
        m_npc->postStartInput(uid, title, commitTag, show);
    });

    bindFunction("uidPostXMLString", [this](uint64_t uid, std::string xmlString)
    {
        fflassert(uid);
        fflassert(uidf::isPlayer(uid), uid, uidf::getUIDString(uid));
        m_npc->postXMLLayout(uid, std::move(xmlString));
    });

    bindFunction("sendCallStackRemoteCall", [this](uint64_t callStackUID, uint64_t uid, std::string code)
    {
        m_npc->sendRemoteCall(callStackUID, uid, code);
    });

    bindFunction("pollCallStackEvent", [this](uint64_t uid, sol::this_state s)
    {
        sol::state_view sv(s);
        return sol::as_returns([uid, &sv, this]() -> std::vector<sol::object>
        {
            if(auto p = m_callStackList.find(uid); p != m_callStackList.end()){
                const auto from = p->second.from;
                const auto event = std::move(p->second.event);
                const auto value = std::move(p->second.value);

                p->second.clearEvent();
                if(!from){
                    return {};
                }

                fflassert(str_haschar(event));
                std::vector<sol::object> eventStack
                {
                    sol::object(sv, sol::in_place_type<uint64_t>, from),
                    sol::object(sv, sol::in_place_type<std::string>, event),
                };

                if(event == SYS_EXECDONE){
                    fflassert(value.has_value());
                    const auto sdLCR = cerealf::deserialize<SDRemoteCallResult>(value.value());
                    if(sdLCR.error.empty()){
                        for(const auto &s: sdLCR.serVarList){
                            eventStack.push_back(luaf::buildLuaObj(sv, s));
                        }
                    }
                    else{
                        fflassert(sdLCR.serVarList.empty(), sdLCR.error, sdLCR.serVarList);
                        for(const auto &line: sdLCR.error){
                            g_monoServer->addLog(LOGTYPE_WARNING, "%s", to_cstr(line));
                        }
                        throw fflerror("lua call failed in %s", to_cstr(uidf::getUIDString(from)));
                    }
                }
                else if(value.has_value()){
                    eventStack.push_back(sol::object(sv, sol::in_place_type<std::string>, value.value()));
                }
                return eventStack;
            }
            throw fflerror("can't find call stack UID = %llu", to_llu(uid));
        }());
    });

    execRawString(BEGIN_LUAINC(char)
#include "npchar.lua"
    END_LUAINC());

    execFile(scriptName.c_str());
    execString
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

    auto p = m_callStackList.find(callStackUID);
    if(p == m_callStackList.end()){
        p = m_callStackList.insert({callStackUID, LuaNPCModule::LuaCallStack(this)}).first;

        // initial call to make main reaches its event polling point
        // need to assign event to let it advance
        pfrCheck(p->second.runner.callback(callStackUID));
    }

    if(!p->second.runner.callback){
        throw fflerror("lua coroutine is not callable");
    }

    // setup the event
    // event may get consumed if coroutine yields in pollCallStackEvent()

    p->second.from  = from;
    p->second.event = std::move(event);
    p->second.value = std::move(value);

    // comsume the event
    // call the coroutine to make it fail or stuck at next pollCallStackEvent()

    if(!pfrCheck(p->second.runner.callback())){
        throw fflerror("lua coroutine run failed");
    }

    if(!p->second.runner.callback){
        m_callStackList.erase(p);
    }
}

NPChar::NPChar(const ServerMap *mapCPtr, const SDInitNPChar &initNPChar)
    : CharObject
      {
          mapCPtr,
          uidf::buildNPCUID(initNPChar.lookID),

          initNPChar.x,
          initNPChar.y,
          initNPChar.gfxDir + DIR_BEGIN, // NPC gfx dir, may not be the 8-dir, but should be in DIR_BEGIN + [0, 8)
      }
    , m_npcName(initNPChar.npcName)
{
    m_luaModulePtr = std::make_unique<LuaNPCModule>(this, initNPChar.fullScriptName);
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

void NPChar::sendRemoteCall(uint64_t callStackUID, uint64_t uid, const std::string &code)
{
    const auto seqID = m_luaModulePtr->getCallStackSeqID(callStackUID);
    if(!seqID){
        throw fflerror("calling sendRemoteCall(%llu, %llu, %s) outside of LuaCallStack", to_llu(callStackUID), to_llu(uid), to_cstr(code));
    }

    m_actorPod->forward(uid, {AM_REMOTECALL, cerealf::serialize(SDRemoteCall
    {
        .code = code,
    })},

    [callStackUID, uid, seqID, code /* not ref */, this](const ActorMsgPack &mpk)
    {
        if(uid != mpk.from()){
            throw fflerror("lua code sent to uid %llu but get response from %llu", to_llu(uid), to_llu(mpk.from()));
        }

        if(mpk.seqID()){
            throw fflerror("call of lua code expects response");
        }

        if(m_luaModulePtr->getCallStackSeqID(callStackUID) != seqID){
            return;
        }

        switch(mpk.type()){
            case AM_SDBUFFER:
                {
                    m_luaModulePtr->setEvent(callStackUID, uid, SYS_EXECDONE, std::string(reinterpret_cast<const char *>(mpk.data()), mpk.size()));
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
