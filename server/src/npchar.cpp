#include <cstdint>
#include "uidf.hpp"
#include "aesf.hpp"
#include "luaf.hpp"
#include "xmlf.hpp"
#include "dbpod.hpp"
#include "mathf.hpp"
#include "npchar.hpp"
#include "colorf.hpp"
#include "totype.hpp"
#include "base64f.hpp"
#include "filesys.hpp"
#include "dbcomid.hpp"
#include "cerealf.hpp"
#include "fflerror.hpp"
#include "serdesmsg.hpp"
#include "friendtype.hpp"
#include "monoserver.hpp"
#include "serverpasswordwindow.hpp"
#include "serverconfigurewindow.hpp"

extern DBPod *g_dbPod;
extern MonoServer *g_monoServer;
extern ServerPasswordWindow *g_serverPasswordWindow;
extern ServerConfigureWindow *g_serverConfigureWindow;

NPChar::AESHelper::AESHelper(const NPChar *npc, uint64_t uid)
    : aesf::AES(g_serverPasswordWindow->getPassword(), reinterpret_cast<uintptr_t>(to_cvptr(npc)) ^ ~uid, npc->getXMLSeqID(uid).value())
{}

std::string NPChar::AESHelper::encode(const char *s)
{
    fflassert(str_haschar(s), s);
    std::string buf(s);

    buf.resize((buf.size() + 15) / 16 * 16);
    encrypt(buf);

    return base64f::encode(buf);
}

std::string NPChar::AESHelper::decode(const char *s)
{
    auto buf = base64f::decode(to_sv(s));
    decrypt(buf);

    while(!(buf.empty() || buf.back() != '\0')){
        buf.pop_back();
    }
    return buf;
}

NPChar::LuaThreadRunner::LuaThreadRunner(NPChar *npc)
    : CharObject::LuaThreadRunner(npc)
{
    fflassert(dynamic_cast<NPChar *>(getSO()));
    fflassert(dynamic_cast<NPChar *>(getSO()) == npc);

    bindFunction("setNPCSell", [this](sol::as_table_t<std::vector<std::string>> itemNameList)
    {
        getNPChar()->m_npcSell.clear();
        getNPChar()->m_sellItemList.clear();

        for(const auto &itemName: itemNameList.value()){
            if(const auto itemID = DBCOM_ITEMID(to_u8cstr(itemName))){
                getNPChar()->m_npcSell.insert(itemID);
            }
        }

        getNPChar()->fillSellItemList();
    });

    bindFunction("addNPCSell", [this](std::string itemName)
    {
        if(const auto itemID = DBCOM_ITEMID(to_u8cstr(itemName))){
            getNPChar()->m_npcSell.insert(itemID);
        }
    });

    bindFunction("clearNPCSell", [this]()
    {
        getNPChar()->m_npcSell.clear();
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
            return getNPChar()->getNPCName().substr(0, getNPChar()->getNPCName().find('_'));
        }
        else{
            return getNPChar()->getNPCName();
        }
    });

    bindFunction("getNPCFullName", [this]() -> std::string
    {
        return std::string(to_cstr(DBCOM_MAPRECORD(getNPChar()->mapID()).name)) + "." + getNPChar()->getNPCName();
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

        const std::string mapName = to_cstr(DBCOM_MAPRECORD(getNPChar()->mapID()).name);
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
        getNPChar()->postAddMonster(monsterID);
    });

    bindFunction("dbSetGKey", [this](std::string key, sol::object obj)
    {
        const auto npcDBName = str_printf("tbl_global_npcdb_%s_%s", to_cstr(DBCOM_MAPRECORD(getNPChar()->mapID()).name), getNPChar()->getNPCName().c_str());
        if(!g_dbPod->createQuery(u8R"###(select name from sqlite_master where type='table' and name='%s')###", npcDBName.c_str()).executeStep()){
            g_dbPod->exec(
                    u8R"###( create table %s(                         )###"
                    u8R"###(     fld_key   text not null primary key, )###"
                    u8R"###(     fld_value blob not null              )###"
                    u8R"###( );                                       )###", npcDBName.c_str());
        }

        auto query = g_dbPod->createQuery(u8R"###(replace into %s(fld_key, fld_value) values('%s', ?))###", npcDBName.c_str(), key.c_str());
        query.bindBlob(1, cerealf::serialize(luaf::buildLuaVar(obj)));
        query.exec();
    });

    bindFunction("dbGetGKey", [this](std::string key, sol::this_state s) -> sol::object
    {
        fflassert(!key.empty());
        const auto npcDBName = str_printf("tbl_global_npcdb_%s_%s", to_cstr(DBCOM_MAPRECORD(getNPChar()->mapID()).name), getNPChar()->getNPCName().c_str());

        sol::state_view sv(s);
        if(!g_dbPod->createQuery(u8R"###(select name from sqlite_master where type='table' and name='%s')###", npcDBName.c_str()).executeStep()){
            return sol::make_object(sv, sol::nil);
        }

        auto queryStatement = g_dbPod->createQuery(u8R"###(select fld_value from %s where fld_key='%s')###", npcDBName.c_str(), key.c_str());
        if(!queryStatement.executeStep()){
            return sol::make_object(sv, sol::nil);
        }
        return luaf::buildLuaObj(sv, cerealf::deserialize<luaf::luaVar>(queryStatement.getColumn(0).getString()));
    });

    bindFunction("uidDBSetKey", [this](uint64_t uid, std::string key, sol::object obj)
    {
        const auto dbid = uidf::getPlayerDBID(uid);
        const auto npcDBName = str_printf("tbl_npcdb_%s_%s", to_cstr(DBCOM_MAPRECORD(getNPChar()->mapID()).name), getNPChar()->getNPCName().c_str());

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
        const auto npcDBName = str_printf("tbl_npcdb_%s_%s", to_cstr(DBCOM_MAPRECORD(getNPChar()->mapID()).name), getNPChar()->getNPCName().c_str());

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
        getNPChar()->postSell(uid);
    });

    bindFunction("uidPostStartInvOp", [this](uint64_t uid, int invOp, std::string queryTag, std::string commitTag, sol::as_table_t<std::vector<std::string>> typeTable)
    {
        fflassert(invOp >= INVOP_BEGIN);
        fflassert(invOp <  INVOP_END);

        std::set<std::u8string> typeList;
        for(const auto &type: typeTable.value()){
            typeList.insert(to_u8cstr(type));
        }
        getNPChar()->postStartInvOp(uid, invOp, queryTag, commitTag, {typeList.begin(), typeList.end()});
    });

    bindFunction("uidPostInvOpCost", [this](uint64_t uid, int invOp, int itemID, int seqID, int cost)
    {
        fflassert(invOp >= INVOP_BEGIN);
        fflassert(invOp <  INVOP_END);

        getNPChar()->postInvOpCost(uid, invOp, itemID, seqID, cost);
    });

    bindFunction("uidPostStartInput", [this](uint64_t uid, std::string title, std::string commitTag, bool show)
    {
        fflassert(!title.empty());
        fflassert(!commitTag.empty());
        getNPChar()->postStartInput(uid, title, commitTag, show);
    });

    bindFunction("uidPostXMLString", [this](uint64_t uid, std::string path, std::string xmlString)
    {
        fflassert(uid);
        fflassert(uidf::isPlayer(uid), uid, uidf::getUIDString(uid));
        getNPChar()->postXMLLayout(uid, std::move(path), std::move(xmlString));
    });

    pfrCheck(execRawString(BEGIN_LUAINC(char)
#include "npchar.lua"
    END_LUAINC()));

    pfrCheck(execFile(getNPChar()->m_initScriptName.c_str()));
    pfrCheck(execRawString(R"###(
        -- sanity check
        -- print warning message for NPCs that have not script installed

        if not hasEventHandler() then
            addLog(LOGTYPE_WARNING, 'No event handler installed: %s', getNPCFullName())
        end
    )###"));
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
    , m_initScriptName(initNPChar.fullScriptName)
{}

uint64_t NPChar::rollXMLSeqID(uint64_t uid)
{
    const auto added = static_cast<uint64_t>(std::rand());
    if(auto p = m_xmlLayoutSeqIDList.find(uid); p != m_xmlLayoutSeqIDList.end()){
        return p->second += added;
    }
    else{
        return m_xmlLayoutSeqIDList.emplace(uid, m_xmlLayoutSeqID += added).first->second;
    }
}

std::optional<uint64_t> NPChar::getXMLSeqID(uint64_t uid) const
{
    if(auto p = m_xmlLayoutSeqIDList.find(uid); p != m_xmlLayoutSeqIDList.end()){
        return p->second;
    }
    else{
        return std::nullopt;
    }
}

bool NPChar::update()
{
    return true;
}

void NPChar::reportCO(uint64_t)
{
}

void NPChar::onActivate()
{
    CharObject::onActivate();
    m_luaRunner = std::make_unique<NPChar::LuaThreadRunner>(this);
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
        .itemList = std::vector<uint32_t>(getSellList().begin(), getSellList().end()),
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

void NPChar::postXMLLayout(uint64_t uid, std::string path, std::string xmlString)
{
    tinyxml2::XMLDocument xmlDoc(true, tinyxml2::PEDANTIC_WHITESPACE);
    if(xmlDoc.Parse(xmlString.c_str()) != tinyxml2::XML_SUCCESS){
        throw fflerror("parse xml failed: %s", to_cstr(xmlString));
    }

    fflassert(xmlDoc.RootElement(), xmlString);
    fflassert(str_tolower(xmlDoc.RootElement()->Name()) == "layout", xmlString);

    auto fnXMLTran = [uid, npc = this](this auto &&self, tinyxml2::XMLNode *node)
    {
        if(!node){
            return;
        }
        if(auto elem = node->ToElement()){
            for(auto attr = elem->FirstAttribute(); attr; attr = attr->Next()){
                if(str_tolower(attr->Name()) == "id"){
                    const_cast<tinyxml2::XMLAttribute *>(attr)->SetAttribute(AESHelper(npc, uid).encode(attr->Value()).c_str());
                    break;
                }
            }
        }

        for(auto child = node->FirstChild(); child; child = child->NextSibling()){
            self(child);
        }
    };

    rollXMLSeqID(uid);
    fnXMLTran(xmlDoc.RootElement());
    forwardNetPackage(uid, SM_NPCXMLLAYOUT, cerealf::serialize(SDNPCXMLLayout
    {
        .npcUID = UID(),
        .eventPath = std::move(path),
        .xmlLayout = xmlf::toString(xmlDoc.RootElement()),
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
        case AM_REMOTECALL:
            {
                on_AM_REMOTECALL(mpk);
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
    for(const uint32_t itemID: getSellList()){
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
                SDItem::build_EA_COLOR(colorf::RGBA(mathf::rand(100, 255), mathf::rand(100, 255), mathf::rand(100, 255), 0XFF)),
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
                SDItem::build_EA_DC(mathf::rand<int>(1, 5)),
                SDItem::build_EA_MC(mathf::rand<int>(1, 5)),
                SDItem::build_EA_SC(mathf::rand<int>(1, 5)),

                SDItem::build_EA_BUFFID([itemID]() -> uint32_t
                {
                    switch(itemID){
                        case DBCOM_ITEMID(u8"龙纹剑"): return DBCOM_BUFFID(u8"龙纹圣光");
                        default                      : return DBCOM_BUFFID(u8"死亡威慑");
                    }
                }()),
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
                SDItem::build_EA_DC(mathf::rand<int>(1, 5)),
                SDItem::build_EA_BUFFID(DBCOM_BUFFID(u8"吸血鬼的诅咒")),
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
                SDItem::build_EA_AC(mathf::rand<int>(1, 5)),
                SDItem::build_EA_MAC(mathf::rand<int>(1, 5)),
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
