#include <cinttypes>
#include "dbpod.hpp"
#include "player.hpp"
#include "luaf.hpp"
#include "uidf.hpp"
#include "uidsf.hpp"
#include "jobf.hpp"
#include "pathf.hpp"
#include "mathf.hpp"
#include "totype.hpp"
#include "dbcomid.hpp"
#include "deliverydb.hpp"
#include "chatdb.hpp"
#include "golddb.hpp"
#include "inventorydb.hpp"
#include "directtradedb.hpp"
#include "sysconst.hpp"
#include "charobject.hpp"
#include "friendtype.hpp"
#include "protocoldef.hpp"
#include "buildconfig.hpp"
#include "serverargparser.hpp"

extern DBPod *g_dbPod;
extern Server *g_server;
extern ServerArgParser *g_serverArgParser;

Player::LuaThreadRunner::LuaThreadRunner(Player *playerPtr)
    : BattleObject::LuaThreadRunner(playerPtr)
{
    bindFunction("getLevel", [this]() -> uint64_t
    {
        return getPlayer()->level();
    });

    bindFunction("rollKey", [this]() -> uint64_t
    {
        return getPlayer()->m_threadKey++;
    });

    // legacy checkmagic: has this player already studied it
    bindFunction("hasMagic", [this](std::string magicName) -> bool
    {
        const auto magicID = DBCOM_MAGICID(magicName.c_str());
        fflassert(magicID, magicName);

        return getPlayer()->m_sdLearnedMagicList.has(magicID);
    });

    bindFunction("getGold", [this]() -> uint64_t
    {
        return getPlayer()->gold();
    });

    // charge a fee, false when the player can not cover it and nothing is taken
    //
    // gold does not sit in the inventory so removeItem can not reach it
    bindFunction("removeGold", [this](int count) -> bool
    {
        fflassert(count > 0);
        if(getPlayer()->gold() < to_uz(count)){
            return false;
        }

        getPlayer()->setGold(getPlayer()->gold() - to_uz(count));
        return true;
    });

    bindFunction("getGender", [this]() -> bool
    {
        return getPlayer()->gender();
    });

    // 战士 / 道士 / 法师, a list because a character can hold more than one
    bindFunction("getJobList", [this](sol::this_state s)
    {
        std::vector<std::string> result;
        for(const auto jobstr: jobf::jobName(getPlayer()->job())){
            result.push_back(to_cstr(jobstr));
        }
        return sol::make_object(sol::state_view(s), result);
    });

    bindFunction("getName", [this]() -> std::string
    {
        return getPlayer()->name();
    });

    bindFunction("getRedName", [this]() -> bool
    {
        return getPlayer()->redName();
    });

    bindFunction("getWLItem", [this](int wlType, sol::this_state s) -> sol::object
    {
        if(const auto &item = getPlayer()->m_sdItemStorage.wear.getWLItem(wlType)){
            return luaf::buildLuaObj(sol::state_view(s), item.asLuaVar());
        }
        else{
            return sol::make_object(sol::state_view(s), sol::lua_nil);
        }
    });

    bindFunction("getTeamLeader", [this](sol::this_state s) -> sol::object
    {
        sol::state_view sv(s);
        if(getPlayer()->m_teamLeader){
            return sol::object(sv, sol::in_place_type<lua_Integer>, getPlayer()->m_teamLeader);
        }
        else{
            return sol::make_object(sv, sol::lua_nil);
        }
    });

    bindCoop("_RSVD_NAME_getTeamMemberList", [thisptr = this](this auto, LuaCoopResumer onDone) -> corof::awaitable<>
    {
        bool closed = false;
        onDone.pushOnClose([&closed](){ closed = true; });

        const auto sdTMLOpt = co_await thisptr->getPlayer()->pullTeamMemberList();
        if(closed){
            co_return;
        }

        onDone.popOnClose();
        if(sdTMLOpt.has_value()){
            onDone(sol::as_table(sdTMLOpt.value().getUIDList()));
        }
        else{
            onDone();
        }
    });

    bindFunction("postRawString", [this](std::string msg)
    {
        getPlayer()->postNetMessage(SM_TEXT, msg);
    });

    bindFunction("secureItem", [this](uint32_t itemID, uint32_t seqID)
    {
        getPlayer()->secureItem(itemID, seqID);
    });

    bindFunction("reportSecuredItemList", [this]()
    {
        getPlayer()->reportSecuredItemList();
    });

    bindFunction("repairItem", [this](uint32_t itemID, uint32_t seqID, bool special) -> bool
    {
        return getPlayer()->repairInventoryItem(itemID, seqID, special);
    });

    // returns {currDuration, maxDuration}, or nil if the item doesn't exist or has no durability
    bindFunction("queryItemDuration", [this](uint32_t itemID, uint32_t seqID, sol::this_state s) -> sol::object
    {
        if(const auto &ir = DBCOM_ITEMRECORD(itemID); ir && !ir.isGold()){
            if(const auto item = getPlayer()->findInventoryItem(itemID, seqID); item && item->duration[1] > 0){
                return luaf::buildLuaObj(sol::state_view(s), luaf::buildLuaVar(std::array<int, 2>
                {
                    to_d(item->duration[0]),
                    to_d(item->duration[1]),
                }));
            }
        }
        return sol::make_object(sol::state_view(s), sol::lua_nil);
    });

    bindFunction("addItem", [this](int itemID, int itemCount)
    {
        const auto &ir = DBCOM_ITEMRECORD(itemID);
        fflassert(ir);
        fflassert(itemCount > 0);

        for(auto item: SDItem::buildItemList(to_u32(itemID), to_uz(itemCount))){
            if(item.isGold()){
                getPlayer()->setGold(getPlayer()->gold() + item.count);
            }
            else{
                getPlayer()->addInventoryItem(std::move(item), false);
            }
        }
    });

    // hand over an item the player can not take off once worn, see SDItem::EA_BIND
    //
    // returns the seqID, a quest needs it to tell the lent copy from one the player already
    // had, and to take the right one back
    bindFunction("addBoundItem", [this](int itemID) -> uint32_t
    {
        const auto &ir = DBCOM_ITEMRECORD(itemID);
        fflassert(ir);

        auto itemList = SDItem::buildItemList(to_u32(itemID), 1);
        fflassert(itemList.size() == 1, itemList.size());

        auto item = itemList.front();
        fflassert(!item.isGold());

        item.extAttrList.insert(SDItem::build_EA_BIND(true));
        return getPlayer()->addInventoryItem(std::move(item), false).seqID;
    });

    // take a worn item away outright, the one path EA_BIND does not block
    bindFunction("removeWearItem", [this](int wltype) -> bool
    {
        if(!getPlayer()->m_sdItemStorage.wear.getWLItem(wltype)){
            return false;
        }

        getPlayer()->setWLItem(wltype, {});
        getPlayer()->dbRemoveWearItem(wltype);

        // same teardown the manual disarm does, so anything hooked on the slot lets go
        if(auto cbp = getPlayer()->m_onWLOff.find(wltype); cbp != getPlayer()->m_onWLOff.end()){
            fflassert(cbp->second);
            cbp->second();
            getPlayer()->m_onWLOff.erase(cbp);
        }
        return true;
    });

    bindFunction("deliverItem", [this](int itemID, int itemCount) -> std::string
    {
        fflassert(itemID > 0);
        fflassert(itemCount > 0);
        return getPlayer()->sendDelivery(SDItem::buildItemList(to_u32(itemID), to_uz(itemCount)));
    });

    bindFunction("removeItem", [this](int itemID, int seqID, int count) -> bool
    {
        fflassert(itemID >  0, itemID);
        fflassert( seqID >= 0,  seqID);
        fflassert( count >  0,  count);

        const auto argItemID = to_u32(itemID);
        const auto argSeqID  = to_u32( seqID);
        const auto argCount  = to_uz ( count);

        const auto &ir = DBCOM_ITEMRECORD(argItemID);
        fflassert(ir);

        if(ir.isGold()){
            fflassert(argSeqID == 0, argSeqID);
            if(getPlayer()->m_sdItemStorage.gold >= argCount){
                getPlayer()->setGold(getPlayer()->m_sdItemStorage.gold - argCount);
                return true;
            }
            else{
                return false;
            }
        }
        else if(argSeqID > 0){
            fflassert(argCount == 1, argCount);
            return getPlayer()->removeInventoryItem(argItemID, argSeqID) > 0;
        }
        else{
            fflassert(argCount > 0);
            if(getPlayer()->hasInventoryItem(argItemID, argSeqID, argCount)){
                getPlayer()->removeInventoryItem(argItemID, 0, argCount);
                return true;
            }
            else{
                return false;
            }
        }
    });

    // take up to count copies away and say how many that was
    //
    // removeItem is all or nothing, which legacy scripts do not expect: they clear a material
    // out with an oversized take, e.g. take 蛆卵 20 to drop however many the player is holding
    bindFunction("removeUpToItem", [this](int itemID, int count) -> size_t
    {
        fflassert(itemID > 0, itemID);
        fflassert( count > 0,  count);

        const auto &ir = DBCOM_ITEMRECORD(itemID);
        fflassert(ir);
        fflassert(!ir.isGold());

        return getPlayer()->removeInventoryItem(to_u32(itemID), 0, to_uz(count));
    });

    bindFunction("hasItem", [this](int itemID, int seqID, size_t count) -> bool
    {
        fflassert(itemID >  0, itemID);
        fflassert( seqID >= 0,  seqID);
        fflassert( count >  0,  count);

        return getPlayer()->hasInventoryItem(to_u32(itemID), to_u32(seqID), count);
    });

    bindFunction("dbGetVar", [this](std::string var, sol::this_state s)
    {
        return luaf::buildLuaObj(sol::state_view(s), getPlayer()->dbGetVar(var));
    });

    bindFunction("dbSetVar", [this](std::string var, sol::object value)
    {
        getPlayer()->dbSetVar(var, luaf::buildLuaVar(value));
    });

    bindFunction("dbHasVar", [this](std::string var, sol::this_state s)
    {
        auto &&[found, value] = getPlayer()->dbHasVar(var);

        sol::state_view sv(s);
        std::vector<sol::object> resList;

        if(found){
            resList.reserve(2);
            resList.push_back(sol::object(sv, sol::in_place_type<bool>, true));
            resList.push_back(luaf::buildLuaObj(sv, std::move(value)));
        }
        else{
            resList.reserve(1);
            resList.push_back(sol::object(sv, sol::in_place_type<bool>, false));
        }

        return sol::as_returns(resList);
    });

    bindFunction("dbRemoveVar", [this](std::string var)
    {
        getPlayer()->dbRemoveVar(std::move(var));
    });

    bindFunction("_RSVD_NAME_reportQuestDespList", [this](sol::object obj)
    {
        fflassert(obj.is<sol::table>(), luaf::luaObjTypeString(obj));
        SDQuestDespList sdQDL {};

        for(const auto &[quest, table]: obj.as<sol::table>()){
            fflassert(quest.is<std::string>(), luaf::luaObjTypeString(quest));
            fflassert(table.is<sol::table >(), luaf::luaObjTypeString(table));

            for(const auto &[fsm, desp]: table.as<sol::table>()){
                fflassert(fsm .is<std::string>(), luaf::luaObjTypeString(fsm ));
                fflassert(desp.is<std::string>(), luaf::luaObjTypeString(desp));
                sdQDL[quest.as<std::string>()][fsm.as<std::string>()] = desp.as<std::string>();
            }
        }

        getPlayer()->postNetMessage(SM_QUESTDESPLIST, cerealf::serialize(sdQDL));
    });

    bindCoop("_RSVD_NAME_spaceMove", [thisptr = this](this auto, LuaCoopResumer onDone, uint32_t argMapID, int argX, int argY) -> corof::awaitable<>
    {
        bool closed = false;
        onDone.pushOnClose([&closed](){ closed = true; });

        const auto &mr = DBCOM_MAPRECORD(argMapID);
        fflassert(mr, argMapID);

        fflassert(argX >= 0, argX);
        fflassert(argY >= 0, argY);

        if(to_u32(argMapID) == thisptr->getPlayer()->mapID()){
            const auto moved = co_await thisptr->getPlayer()->requestSpaceMove(argX, argY, false);
            if(closed){
                co_return;
            }

            onDone.popOnClose();
            if(moved){
                onDone(thisptr->getPlayer()->mapID(), thisptr->getPlayer()->X(), thisptr->getPlayer()->Y());
            }
            else{
                onDone();
            }
        }
        else{
            const auto switched = co_await thisptr->getPlayer()->requestMapSwitch(uidsf::getMapBaseUID(argMapID), argX, argY, false);
            if(closed){
                co_return;
            }

            onDone.popOnClose();
            if(switched){
                onDone(thisptr->getPlayer()->mapID(), thisptr->getPlayer()->X(), thisptr->getPlayer()->Y());
            }
            else{
                onDone();
            }
        }
    });

    // move into a map addressed by uid, which is the only way into an instance copy
    //
    // _RSVD_NAME_spaceMove takes a map id and always resolves it to that map's base copy, so a
    // script holding an instance uid can not get there through it
    bindCoop("_RSVD_NAME_mapUIDMove", [thisptr = this](this auto, LuaCoopResumer onDone, uint64_t argMapUID, int argX, int argY) -> corof::awaitable<>
    {
        bool closed = false;
        onDone.pushOnClose([&closed](){ closed = true; });

        fflassert(uidf::isMap(argMapUID), uidf::getUIDString(argMapUID));

        fflassert(argX >= 0, argX);
        fflassert(argY >= 0, argY);

        const auto switched = co_await thisptr->getPlayer()->requestMapSwitch(argMapUID, argX, argY, false);
        if(closed){
            co_return;
        }

        onDone.popOnClose();
        if(switched){
            onDone(thisptr->getPlayer()->mapID(), thisptr->getPlayer()->X(), thisptr->getPlayer()->Y());
        }
        else{
            onDone();
        }
    });

    bindCoop("_RSVD_NAME_randomMove", [thisptr = this](this auto, LuaCoopResumer onDone) -> corof::awaitable<>
    {
        const auto newGLocOpt = [thisptr]() -> std::optional<std::pair<int, int>>
        {
            for(int startDir = pathf::getRandDir(), i = 0; i < 8; ++i){
                if(const auto [newX, newY] = pathf::getFrontGLoc(thisptr->getPlayer()->X(), thisptr->getPlayer()->Y(), pathf::getNextDir(startDir, i)); thisptr->getPlayer()->mapBin()->groundValid(newX, newY)){
                    return std::make_pair(newX, newY);
                }
            }
            return std::nullopt;
        }();

        if(!newGLocOpt.has_value()){
            onDone();
            co_return;
        }

        bool closed = false;
        onDone.pushOnClose([&closed](){ closed = true; });

        const auto oldX = thisptr->getPlayer()->X();
        const auto oldY = thisptr->getPlayer()->Y();
        const auto [newX, newY] = newGLocOpt.value();

        const auto moved = co_await thisptr->getPlayer()->requestMove(newX, newY, SYS_DEFSPEED, false, false);
        if(closed){
            co_return;
        }

        onDone.popOnClose();

        // player doesn't sendback its move to client in requestMove() because player's move usually driven by client
        // but here need to sendback the forced move since it's driven by server

        if(moved){
            thisptr->getPlayer()->reportAction(thisptr->getPlayer()->UID(), thisptr->getPlayer()->mapUID(), ActionMove
            {
                .speed = SYS_DEFSPEED,
                .x = oldX,
                .y = oldY,
                .aimX = thisptr->getPlayer()->X(),
                .aimY = thisptr->getPlayer()->Y(),
            });

            onDone(thisptr->getPlayer()->mapID(), thisptr->getPlayer()->X(), thisptr->getPlayer()->Y());
        }
        else{
            onDone();
        }
    });

    bindCoop("_RSVD_NAME_queryQuestTriggerList", [thisptr = this](this auto, LuaCoopResumer onDone, int triggerType) -> corof::awaitable<>
    {
        fflassert(triggerType >= SYS_ON_BEGIN, triggerType);
        fflassert(triggerType <  SYS_ON_END  , triggerType);

        bool closed = false;
        onDone.pushOnClose([&closed](){ closed = true; });

        AMQueryQuestTriggerList amQQTL;
        std::memset(&amQQTL, 0, sizeof(amQQTL));
        amQQTL.type = triggerType;

        const auto rmpk = co_await thisptr->getPlayer()->m_actorPod->send(uidf::getServiceCoreUID(), {AM_QUERYQUESTTRIGGERLIST, amQQTL});
        if(closed){
            co_return;
        }

        onDone.popOnClose();
        switch(rmpk.type()){
            case AM_OK:
                {
                    onDone(rmpk.template deserialize<std::vector<uint64_t>>());
                    break;
                }
            default:
                {
                    onDone();
                    break;
                }
        }
    });

    constexpr static unsigned char luaScript []
    {
        #embed "player.lua" suffix(,)
        '\0'
    };
    pfrCheck(execRawString(to_rawcstr(luaScript)));
}

Player::Player(const SDInitPlayer &initParam)
    : BattleObject(uidf::getPlayerUID(initParam.dbid), initParam.mapUID, initParam.x, initParam.y, DIR_DOWN)
    , m_exp(initParam.exp)
    , m_gender(initParam.gender)
    , m_job(initParam.job)
    , m_name(initParam.name)
    , m_pkPoint(initParam.pkPoint)
    , m_hair(initParam.hair)
    , m_hairColor(initParam.hairColor)
{
    m_sdHealth.uid = UID();
    m_sdHealth.hp = initParam.hp;
    m_sdHealth.mp = initParam.mp;
    m_sdHealth.maxHP = maxHP();
    m_sdHealth.maxMP = maxMP();
    m_sdHealth.hpRecover = 1;
    m_sdHealth.hpRecover = 1;

    m_sdItemStorage.gold = initParam.gold;

    dbLoadWear();
    dbLoadBelt();
    m_sdItemStorage.inventory = dbLoadInventory(dbid());
    dbLoadFriendList();
    dbLoadLearnedMagic();
    dbLoadPlayerConfig();
}

corof::awaitable<> Player::onActivate()
{
    co_await BattleObject::onActivate();
    m_luaRunner = std::make_unique<Player::LuaThreadRunner>(this);
    m_luaRunner->spawn(m_threadKey++, "_RSVD_NAME_setupQuests()");
}

corof::awaitable<> Player::onActorMsg(const ActorMsgPack &mpk)
{
    switch(mpk.type()){
        case AM_BADACTORPOD:
            {
                return on_AM_BADACTORPOD(mpk);
            }
        case AM_NOTIFYNEWCO:
            {
                return on_AM_NOTIFYNEWCO(mpk);
            }
        case AM_QUERYHEALTH:
            {
                return on_AM_QUERYHEALTH(mpk);
            }
        case AM_CHECKMASTER:
            {
                return on_AM_CHECKMASTER(mpk);
            }
        case AM_MAPSWITCHTRIGGER:
            {
                return on_AM_MAPSWITCHTRIGGER(mpk);
            }
        case AM_QUERYLOCATION:
            {
                return on_AM_QUERYLOCATION(mpk);
            }
        case AM_QUERYFRIENDTYPE:
            {
                return on_AM_QUERYFRIENDTYPE(mpk);
            }
        case AM_QUERYREDNAME:
            {
                return on_AM_QUERYREDNAME(mpk);
            }
        case AM_EXP:
            {
                return on_AM_EXP(mpk);
            }
        case AM_ADDBUFF:
            {
                return on_AM_ADDBUFF(mpk);
            }
        case AM_REMOVEBUFF:
            {
                return on_AM_REMOVEBUFF(mpk);
            }
        case AM_MISS:
            {
                return on_AM_MISS(mpk);
            }
        case AM_HEAL:
            {
                return on_AM_HEAL(mpk);
            }
        case AM_ACTION:
            {
                return on_AM_ACTION(mpk);
            }
        case AM_ATTACK:
            {
                return on_AM_ATTACK(mpk);
            }
        case AM_DEADFADEOUT:
            {
                return on_AM_DEADFADEOUT(mpk);
            }
        case AM_BINDCHANNEL:
            {
                return on_AM_BINDCHANNEL(mpk);
            }
        case AM_SENDPACKAGE:
            {
                return on_AM_SENDPACKAGE(mpk);
            }
        case AM_RECVPACKAGE:
            {
                return on_AM_RECVPACKAGE(mpk);
            }
        case AM_QUERYUIDBUFF:
            {
                return on_AM_QUERYUIDBUFF(mpk);
            }
        case AM_QUERYCORECORD:
            {
                return on_AM_QUERYCORECORD(mpk);
            }
        case AM_BADCHANNEL:
            {
                return on_AM_BADCHANNEL(mpk);
            }
        case AM_OFFLINE:
            {
                return on_AM_OFFLINE(mpk);
            }
        case AM_QUERYPLAYERNAME:
            {
                return on_AM_QUERYPLAYERNAME(mpk);
            }
        case AM_QUERYPLAYERWLDESP:
            {
                return on_AM_QUERYPLAYERWLDESP(mpk);
            }
        case AM_REMOVEGROUNDITEM:
            {
                return on_AM_REMOVEGROUNDITEM(mpk);
            }
        case AM_CORECORD:
            {
                return on_AM_CORECORD(mpk);
            }
        case AM_NOTIFYDEAD:
            {
                return on_AM_NOTIFYDEAD(mpk);
            }
        case AM_REMOTECALL:
            {
                return on_AM_REMOTECALL(mpk);
            }
        case AM_REQUESTJOINTEAM:
            {
                return on_AM_REQUESTJOINTEAM(mpk);
            }
        case AM_REQUESTLEAVETEAM:
            {
                return on_AM_REQUESTLEAVETEAM(mpk);
            }
        case AM_QUERYTEAMPLAYER:
            {
                return on_AM_QUERYTEAMPLAYER(mpk);
            }
        case AM_QUERYTEAMMEMBERLIST:
            {
                return on_AM_QUERYTEAMMEMBERLIST(mpk);
            }
        case AM_TEAMUPDATE:
            {
                return on_AM_TEAMUPDATE(mpk);
            }
        case AM_PLAYERSAY:
            {
                return on_AM_PLAYERSAY(mpk);
            }
        case AM_PLAYERBROADCAST:
            {
                return on_AM_PLAYERBROADCAST(mpk);
            }
        case AM_ACCEPTDIRECTTRADE:
            {
                return on_AM_ACCEPTDIRECTTRADE(mpk);
            }
        case AM_REJECTDIRECTTRADE:
            {
                return on_AM_REJECTDIRECTTRADE(mpk);
            }
        case AM_UPDATEDIRECTTRADE:
            {
                return on_AM_UPDATEDIRECTTRADE(mpk);
            }
        case AM_COMPLETEDIRECTTRADE:
            {
                return on_AM_COMPLETEDIRECTTRADE(mpk);
            }
        case AM_DIRECTTRADEERROR:
            {
                return on_AM_DIRECTTRADEERROR(mpk);
            }
        case AM_CANCELDIRECTTRADE:
            {
                return on_AM_CANCELDIRECTTRADE(mpk);
            }
        default:
            {
                throw fflvalue(mpk.str(UID()));
            }
    }
}

corof::awaitable<> Player::operateNet(uint8_t nType, const uint8_t *pData, size_t nDataLen, uint64_t respID)
{
    switch(nType){
#define _support_cm(cm) case cm: return net_##cm(nType, pData, nDataLen, respID)
        _support_cm(CM_ACTION                    );
        _support_cm(CM_BUY                       );
        _support_cm(CM_ADDFRIEND                 );
        _support_cm(CM_ACCEPTADDFRIEND           );
        _support_cm(CM_REJECTADDFRIEND           );
        _support_cm(CM_BLOCKPLAYER               );
        _support_cm(CM_CHATMESSAGE               );
        _support_cm(CM_PLAYERSAY                 );
        _support_cm(CM_PLAYERBROADCAST           );
        _support_cm(CM_CONSUMEITEM               );
        _support_cm(CM_DROPITEM                  );
        _support_cm(CM_MAKEITEM                  );
        _support_cm(CM_NPCEVENT                  );
        _support_cm(CM_PICKUP                    );
        _support_cm(CM_PING                      );
        _support_cm(CM_QUERYCORECORD             );
        _support_cm(CM_QUERYMAPBASEUID           );
        _support_cm(CM_QUERYGOLD                 );
        _support_cm(CM_QUERYPLAYERNAME           );
        _support_cm(CM_QUERYPLAYERWLDESP         );
        _support_cm(CM_QUERYCHATPEERLIST         );
        _support_cm(CM_QUERYCHATMESSAGE          );
        _support_cm(CM_QUERYSELLITEMLIST         );
        _support_cm(CM_QUERYUIDBUFF              );
        _support_cm(CM_REQUESTADDHP              );
        _support_cm(CM_REQUESTADDEXP             );
        _support_cm(CM_REQUESTEQUIPBELT          );
        _support_cm(CM_REQUESTEQUIPWEAR          );
        _support_cm(CM_REQUESTGRABBELT           );
        _support_cm(CM_REQUESTGRABWEAR           );
        _support_cm(CM_REQUESTJOINTEAM           );
        _support_cm(CM_REQUESTDIE                );
        _support_cm(CM_REQUESTKILLPETS           );
        _support_cm(CM_REQUESTLEAVETEAM          );
        _support_cm(CM_REQUESTRETRIEVESECUREDITEM);
        _support_cm(CM_REQUESTLATESTCHATMESSAGE  );
        _support_cm(CM_QUERYRANKING              );
        _support_cm(CM_CLAIMDELIVERY             );
        _support_cm(CM_QUERYAUCTIONITEMLIST      );
        _support_cm(CM_REGISTERAUCTIONITEM       );
        _support_cm(CM_BUYAUCTIONITEM            );
        _support_cm(CM_UNREGISTERAUCTIONITEM     );
        _support_cm(CM_REQUESTSPACEMOVE          );
        _support_cm(CM_SETMAGICKEY               );
        _support_cm(CM_SETRUNTIMECONFIG          );
        _support_cm(CM_CREATECHATGROUP           );
        _support_cm(CM_REQUESTDIRECTTRADE        );
        _support_cm(CM_RESPONDDIRECTTRADE        );
        _support_cm(CM_UPDATEDIRECTTRADE         );
        _support_cm(CM_COMMITDIRECTTRADE         );
        _support_cm(CM_CANCELDIRECTTRADE         );
        default:
            {
                throw fflvalue(ClientMsg(nType).name());
            }
#undef _support_cm
    }
}

void Player::reportCO(uint64_t toUID)
{
    if(!toUID){
        return;
    }

    AMCORecord amCOR;
    std::memset(&amCOR, 0, sizeof(amCOR));

    amCOR.UID = UID();
    amCOR.mapUID = mapUID();
    amCOR.action = makeActionStand();
    amCOR.Player.gender = gender();
    amCOR.Player.job = job();
    amCOR.Player.Level = level();
    m_actorPod->post(toUID, {AM_CORECORD, amCOR});
}

void Player::reportStand()
{
    reportAction(UID(), mapUID(), makeActionStand());
}

void Player::reportAction(uint64_t uid, uint64_t actionMapUID, const ActionNode &action)
{
    fflassert(uid);

    SMAction smA;
    std::memset(&smA, 0, sizeof(smA));

    // player can forward CO's action that not on same map
    // this is used for CO map switch, client use it to remove left neighbors

    smA.UID = uid;
    smA.mapUID = actionMapUID;
    smA.action = action;

    postNetMessage(SM_ACTION, smA);
}

void Player::reportDeadUID(uint64_t nDeadUID)
{
    SMNotifyDead smND;
    std::memset(&smND, 0, sizeof(smND));

    smND.UID = nDeadUID;
    postNetMessage(SM_NOTIFYDEAD, smND);
}

void Player::reportHealth()
{
    dispatchNetPackage(true, SM_HEALTH, cerealf::serialize(m_sdHealth));
}

void Player::reportNextStrike()
{
    postNetMessage(SM_NEXTSTRIKE);
}

void Player::onDie()
{
    m_luaRunner->spawn(m_threadKey++, "_RSVD_NAME_trigger(SYS_ON_DIE)");
    const ActionDie die
    {
        .x = X(),
        .y = Y(),
    };

    dispatchAction(die);
    reportAction(UID(), mapUID(), die);
}

void Player::onRevive()
{
    m_luaRunner->spawn(m_threadKey++, "_RSVD_NAME_trigger(SYS_ON_REVIVE)");
    const auto stand = makeActionStand();

    dispatchAction(stand);
    reportAction(UID(), mapUID(), stand);
}

bool Player::dcValid(int, bool)
{
    return true;
}

DamageNode Player::getAttackDamage(int nDC, int) const
{
    const auto node = getCombatNode(m_sdItemStorage.wear, {}, UID(), level());
    const double elemRatio = 1.0 + 0.1 * [nDC, &node]() -> int
    {
        const auto &mr = DBCOM_MAGICRECORD(nDC);
        fflassert(mr);

        switch(magicElemID(mr.elem)){
            case MET_FIRE   : return node.dcElem.fire;
            case MET_ICE    : return node.dcElem.ice;
            case MET_LIGHT  : return node.dcElem.light;
            case MET_WIND   : return node.dcElem.wind;
            case MET_HOLY   : return node.dcElem.holy;
            case MET_DARK   : return node.dcElem.dark;
            case MET_PHANTOM: return node.dcElem.phantom;
            default         : return 0;
        }
    }();

    switch(nDC){
            case DBCOM_MAGICID(u8"烈火剑法"):
            case DBCOM_MAGICID(u8"翔空剑法"):
            case DBCOM_MAGICID(u8"莲月剑法"):
            case DBCOM_MAGICID(u8"半月弯刀"):
            case DBCOM_MAGICID(u8"十方斩"  ):
            case DBCOM_MAGICID(u8"攻杀剑术"):
            case DBCOM_MAGICID(u8"刺杀剑术"):
            case DBCOM_MAGICID(u8"物理攻击"):
            {
                return PlainPhyDamage
                {
                    .damage = node.randPickDC(),
                    .dcHit = node.dcHit,
                };
            }
        case DBCOM_MAGICID(u8"灵魂火符"):
        case DBCOM_MAGICID(u8"冰月神掌"):
        case DBCOM_MAGICID(u8"冰月震天"):
            {
                return MagicDamage
                {
                    .magicID = nDC,
                    .damage = to_d(std::lround(node.randPickSC() * elemRatio)),
                    .mcHit = node.mcHit,
                };
            }
        case DBCOM_MAGICID(u8"雷电术"):
        case DBCOM_MAGICID(u8"火球术"):
        case DBCOM_MAGICID(u8"大火球"):
        case DBCOM_MAGICID(u8"疾光电影"):
        case DBCOM_MAGICID(u8"地狱火"):
        case DBCOM_MAGICID(u8"冰沙掌"):
            {
                return MagicDamage
                {
                    .magicID = nDC,
                    .damage = to_d(std::lround(node.randPickMC() * elemRatio)),
                    .mcHit = node.mcHit,
                };
            }
        default:
            {
                return {};
            }
    }
}

bool Player::struckDamage(uint64_t fromUID, const DamageNode &node)
{
    if(!node){
        return false;
    }

    const auto damage = [&node, this]() -> int
    {
        const auto combatNode = getCombatNode(m_sdItemStorage.wear, {}, UID(), level());
        if(DBCOM_MAGICID(u8"物理攻击") == to_u32(node.magicID)){
            return std::max<int>(0, node.damage - mathf::rand<int>(combatNode.ac[0], combatNode.ac[1]));
        }

        const double elemRatio = std::max<double>(0.0, 1.0 + 0.1 * [&node, &combatNode, this]() -> int
        {
            const auto &mr = DBCOM_MAGICRECORD(node.magicID);
            fflassert(mr);

            switch(magicElemID(mr.elem)){
                case MET_FIRE   : return combatNode.acElem.fire;
                case MET_ICE    : return combatNode.acElem.ice;
                case MET_LIGHT  : return combatNode.acElem.light;
                case MET_WIND   : return combatNode.acElem.wind;
                case MET_HOLY   : return combatNode.acElem.holy;
                case MET_DARK   : return combatNode.acElem.dark;
                case MET_PHANTOM: return combatNode.acElem.phantom;
                default         : return 0;
            }
        }());
        return std::max<int>(0, node.damage - std::lround(mathf::rand<int>(combatNode.mac[0], combatNode.mac[1]) * elemRatio));
    }();

    // remember who hit me, this is what makes checkFriend() report FT_ENEMY and lets me
    // hit back regardless of my attack mode, see canDamageTarget()
    //
    // record it even when my armor soaked the whole hit: the attempt is what counts, or a
    // heavily armored player could be farmed forever without ever being allowed to hit back

    if(fromUID && uidf::isPlayer(fromUID)){
        addOffenderDamage(fromUID, damage);
    }

    if(damage > 0){
        updateHealth(-damage);
        if(m_sdHealth.dead()){
            onDie();

            // the killer decides whether this was a pk, only it knows if I ever attacked it
            if(fromUID && uidf::isPlayer(fromUID)){
                notifyDead(fromUID);
            }
            return true;
        }
    }

    // armor wears from the blow it takes, not from the damage that gets through,
    // fully soaking a hit is exactly when it should wear

    damageDefendWearItem();
    return true;
}

bool Player::ActionValid(const ActionNode &)
{
    return true;
}

bool Player::directTradeBusy() const
{
    return m_directTradePeerOffer.uid != 0;
}

bool Player::directTradeCoordinator() const
{
    return m_directTradeStarted && UID() < m_directTradePeerOffer.uid;
}

bool Player::directTradeReady() const
{
    return m_directTradeStarted
        && m_directTradeOffer.locked
        && m_directTradePeerOffer.locked;
}

bool Player::directTradeConfirmed() const
{
    return directTradeReady()
        && m_directTradeOffer.confirmed
        && m_directTradePeerOffer.confirmed;
}

void Player::startDirectTrade()
{
    const auto targetUID = m_directTradePeerOffer.uid;
    fflassert(uidf::isPlayer(targetUID));
    fflassert(targetUID != UID());

    m_directTradeStarted = true;
    m_directTradeOffer.clear();
    m_directTradeOffer.uid = UID();
    m_directTradePeerOffer.clear();
    m_directTradePeerOffer.uid = targetUID;
}

void Player::clearDirectTrade()
{
    m_directTradeStarted = false;
    m_directTradeOffer.clear();
    m_directTradePeerOffer.clear();
}

void Player::postDirectTradeError(int error)
{
    SMDirectTradeError smDTE;
    std::memset(&smDTE, 0, sizeof(smDTE));

    smDTE.error = to_u8(error);
    postNetMessage(SM_DIRECTTRADEERROR, smDTE);
}

void Player::commitDirectTrade()
{
    if(!directTradeCoordinator() || !directTradeConfirmed()){
        return;
    }

    if(auto tradeRes = dbCommitDirectTrade(m_directTradeOffer, m_directTradePeerOffer); tradeRes.has_value()){
        fflassert(tradeRes.value().at(0).uid == m_directTradeOffer.uid);
        fflassert(tradeRes.value().at(1).uid == m_directTradePeerOffer.uid);

        m_actorPod->post(m_directTradePeerOffer.uid, {AM_COMPLETEDIRECTTRADE, cerealf::serialize(tradeRes.value().at(1))});
        applyDirectTradeResult(std::move(tradeRes.value().at(0)));
    }
    else{
        AMDirectTradeError amDTE;
        std::memset(&amDTE, 0, sizeof(amDTE));

        amDTE.error = to_u8(tradeRes.error());
        m_actorPod->post(m_directTradePeerOffer.uid, {AM_DIRECTTRADEERROR, amDTE});

        postDirectTradeError(tradeRes.error());
        cancelDirectTrade();
    }
}

void Player::applyDirectTradeResult(SDDirectTradeResult result)
{
    fflassert(result.uid == UID(), result.uid, UID());

    const auto peerUID = m_directTradePeerOffer.uid;
    const auto gainedItemList = m_directTradePeerOffer.itemList;

    m_sdItemStorage.gold = result.gold;
    m_sdItemStorage.inventory = std::move(result.inventory);

    reportGold();
    postNetMessage(SM_INVENTORY, cerealf::serialize(m_sdItemStorage.inventory));

    for(const auto &item: gainedItemList){
        m_luaRunner->spawn(m_threadKey++, str_printf("_RSVD_NAME_trigger(SYS_ON_GAINITEM, %llu)", to_llu(item.itemID)));
    }

    clearDirectTrade();

    SMCompleteDirectTrade smCDT;
    std::memset(&smCDT, 0, sizeof(smCDT));

    smCDT.uid = peerUID;
    postNetMessage(SM_COMPLETEDIRECTTRADE, smCDT);
}

void Player::cancelDirectTrade()
{
    // During an established trade only the lower-UID coordinator decides the result
    // The other player sends a cancellation request and keeps server state until the coordinator replies with cancel or completion

    if(m_directTradePeerOffer.uid && m_directTradeStarted){
        if(directTradeCoordinator()){
            SMCloseDirectTrade smCDT;
            std::memset(&smCDT, 0, sizeof(smCDT));

            smCDT.uid = m_directTradePeerOffer.uid;
            postNetMessage(SM_CLOSEDIRECTTRADE, smCDT);

            m_actorPod->post(m_directTradePeerOffer.uid, AM_CANCELDIRECTTRADE);
            clearDirectTrade();
        }
        else{
            m_actorPod->post(m_directTradePeerOffer.uid, AM_CANCELDIRECTTRADE);
        }
    }
}

void Player::dispatchOffline()
{
    cancelDirectTrade();

    if(hasActorPod()){
        AMOffline amO;
        std::memset(&amO, 0, sizeof(amO));

        amO.UID    = UID();
        amO.mapUID = mapUID();
        amO.X      = X();
        amO.Y      = Y();

        m_actorPod->post(mapUID(), {AM_OFFLINE, amO});
        return;
    }

    g_server->addLog(LOGTYPE_WARNING, "Can't dispatch offline event");
}

void Player::reportOffline(uint64_t nUID, uint64_t nMapUID)
{
    fflassert(nUID);
    fflassert(nMapUID);

    // player can initiatively start the offline procedure
    // in this case the m_channID still contains a good channel id, we need to call close

    if(m_channID.value_or(0)){
        SMOffline smO;
        std::memset(&smO, 0, sizeof(smO));

        smO.UID = nUID;
        smO.mapUID = nMapUID;
        postNetMessage(SM_OFFLINE, smO);
    }
}

bool Player::goOffline()
{
    if(!m_channID.value_or(0)){
        return false;
    }

    dispatchOffline();
    reportOffline(UID(), mapUID()); // report self offline

    m_channID = 0;
    m_actorPod->closeNet(); // blocking call, channel slot has been destroyed

    dbUpdateMapGLoc();
    if(m_sdHealth.dead()){
        setHealth(10);
    }

    m_luaRunner->spawn(m_threadKey++, "_RSVD_NAME_trigger(SYS_ON_OFFLINE)", {}, [this](const sol::protected_function_result &)
    {
        deactivate();
    },

    [this]()
    {
        deactivate();
    });

    return true;
}

void Player::postNetMessage(uint8_t headCode, const void *buf, size_t bufLen, uint64_t respID)
{
    if(m_channID.value_or(0)){
        m_actorPod->postNet(headCode, (const uint8_t *)(buf), bufLen, respID);
    }
}

corof::awaitable<> Player::onCMActionStand(CMAction stCMA)
{
    int nX = stCMA.action.x;
    int nY = stCMA.action.y;
    int nDirection = stCMA.action.direction;

    if(mapBin()->validC(nX, nY)){
        // server get report stand
        // means client is trying to re-sync
        // try client's current location and always response

        switch(estimateHop(nX, nY)){
            case 1:
                {
                    if(co_await requestMove(nX, nY, SYS_MAXSPEED, false, false)){
                        co_await onCMActionStand(stCMA);
                    }
                    else{
                        reportStand();
                    }
                    break;
                }
            case 0:
            default:
                {
                    if(pathf::dirValid(nDirection)){
                        m_direction = nDirection;
                    }

                    reportStand();
                    break;
                }
        }
    }
}

corof::awaitable<> Player::onCMActionMove(CMAction stCMA)
{
    cancelDirectTrade();

    // server won't do any path finding
    // client should sent action with only one-hop movement

    int nX0 = stCMA.action.x;
    int nY0 = stCMA.action.y;
    int nX1 = stCMA.action.aimX;
    int nY1 = stCMA.action.aimY;

    switch(estimateHop(nX0, nY0)){
        case 0:
            {
                if(co_await requestMove(nX1, nY1, moveSpeed(), false, false)){
                    dbUpdateMapGLoc();
                }
                else{
                    reportStand();
                }
                break;
            }
        case 1:
            {
                if(co_await requestMove(nX0, nY0, SYS_MAXSPEED, false, false)){
                    co_await onCMActionMove(stCMA);
                }
                else{
                    reportStand();
                }
                break;
            }
        default:
            {
                reportStand();
                break;
            }
    }
}

corof::awaitable<> Player::onCMActionMine(CMAction stCMA)
{
    // server won't do any path finding
    // client should sent action with only one-hop movement

    const ActionMine mine = stCMA.action;
    switch(mathf::LDistance2(mine.x, mine.y, X(), Y())){
        case 0:
            {
                break;
            }
        case 1:
        case 2:
            {
                if(DBCOM_ITEMRECORD(m_sdItemStorage.wear.getWLItem(WLG_WEAPON)).equip.weapon.mine){
                    dispatchAction(mine);
                    addInventoryItem(SDItem
                    {
                        .itemID = DBCOM_ITEMID(u8"黑铁矿"),
                        .seqID  = 1,
                        .count  = 1,
                    }, false);
                }
                break;
            }
        default:
            {
                break;
            }
    }

    return {};
}

corof::awaitable<> Player::onCMActionAttack(CMAction stCMA)
{
    const auto coLocOpt = co_await getCOLocation(stCMA.action.aimUID);
    if(!coLocOpt.has_value()){
        co_return;
    }

    const auto &coLoc = coLocOpt.value();

    int nX0 = stCMA.action.x;
    int nY0 = stCMA.action.y;

    int nDCType = stCMA.action.extParam.attack.magicID;
    uint64_t nAimUID = stCMA.action.aimUID;

    if(coLoc.mapUID != mapUID()){
        co_return;
    }

    switch(nDCType){
        case DBCOM_MAGICID(u8"烈火剑法"):
        case DBCOM_MAGICID(u8"翔空剑法"):
        case DBCOM_MAGICID(u8"莲月剑法"):
        case DBCOM_MAGICID(u8"半月弯刀"):
        case DBCOM_MAGICID(u8"十方斩"  ):
        case DBCOM_MAGICID(u8"攻杀剑术"):
        case DBCOM_MAGICID(u8"刺杀剑术"):
        case DBCOM_MAGICID(u8"物理攻击"):
            {
                switch(estimateHop(nX0, nY0)){
                    case 0:
                        {
                            if(const auto aimDir = pathf::getOffDir(X(), Y(), coLoc.x, coLoc.y); pathf::dirValid(aimDir)){
                                m_direction = aimDir;
                                dispatchAction(makeActionStand());

                                // don't need to send direction change back to client
                                // it has already turned
                            }

                            switch(mathf::LDistance2(nX0, nY0, coLoc.x, coLoc.y)){
                                case 1:
                                case 2:
                                    {
                                        const auto [buffID, modifierID] = m_buffList.rollAttackModifier();

                                        // client reports 攻杀技术 but server need to validate if it's scheduled
                                        // if not scheduled then dispatch 物理攻击 instead, this is for client anti-cheat
                                        dispatchAction(ActionAttack
                                        {
                                            .speed = stCMA.action.speed,
                                            .x = stCMA.action.x,
                                            .y = stCMA.action.y,
                                            .aimUID = stCMA.action.aimUID,
                                            .extParam
                                            {
                                                .magicID = [nDCType, this]() -> uint32_t
                                                {
                                                    if(to_u32(nDCType) == DBCOM_MAGICID(u8"攻杀剑术") && !m_nextStrike){
                                                        return DBCOM_MAGICID(u8"物理攻击");
                                                    }
                                                    else{
                                                        return nDCType;
                                                    }
                                                }(),
                                                .modifierID = to_u32(modifierID),
                                            },
                                        });

                                        std::vector<uint64_t> aimUIDList;
                                        switch(nDCType){
                                            case DBCOM_MAGICID(u8"莲月剑法"):
                                                {
                                                    aimUIDList.push_back(nAimUID);
                                                    aimUIDList.push_back(nAimUID); // attack twice
                                                    break;
                                                }
                                            case DBCOM_MAGICID(u8"半月弯刀"):
                                                {
                                                    std::inplace_vector<std::tuple<int, int>, 3> aimGridList;
                                                    for(int d: {-1, 0, 1}){
                                                        aimGridList.push_back(pathf::getFrontGLoc(X(), Y(), pathf::getNextDir(Direction(), d)));
                                                    }

                                                    for(const auto &[uid, coLoc]: m_inViewCOList){
                                                        if(std::find(aimGridList.begin(), aimGridList.end(), std::make_tuple(coLoc.x, coLoc.y)) != aimGridList.end()){
                                                            aimUIDList.push_back(uid);
                                                        }
                                                    }
                                                    break;
                                                }
                                            case DBCOM_MAGICID(u8"十方斩"):
                                                {
                                                    for(const auto &[uid, coLoc]: m_inViewCOList){
                                                        if(mathf::CDistance<int>(X(), Y(), coLoc.x, coLoc.y) <= 1){
                                                            aimUIDList.push_back(uid);
                                                        }
                                                    }
                                                    break;
                                                }
                                            case DBCOM_MAGICID(u8"刺杀剑术"):
                                                {
                                                    std::array<std::tuple<int, int>, 2> aimGridList
                                                    {
                                                        pathf::getFrontGLoc(X(), Y(), Direction(), 1),
                                                        pathf::getFrontGLoc(X(), Y(), Direction(), 2),
                                                    };

                                                    for(const auto &[uid, coLoc]: m_inViewCOList){
                                                        if(std::find(aimGridList.begin(), aimGridList.end(), std::make_tuple(coLoc.x, coLoc.y)) != aimGridList.end()){
                                                            aimUIDList.push_back(uid);
                                                        }
                                                    }
                                                    break;
                                                }
                                            case DBCOM_MAGICID(u8"翔空剑法"):
                                            case DBCOM_MAGICID(u8"攻杀剑术"):
                                            case DBCOM_MAGICID(u8"烈火剑法"):
                                            case DBCOM_MAGICID(u8"物理攻击"):
                                            default:
                                                {
                                                    aimUIDList.push_back(nAimUID);
                                                    break;
                                                }
                                        }

                                        for(const auto uid: aimUIDList){
                                            if(!co_await canDamageTarget(uid)){
                                                continue;
                                            }

                                            if(buffID){
                                                sendBuff(uid, 0, buffID);
                                            }
                                            markAggression(co_await queryPlayerController(uid));
                                            dispatchAttackDamage(uid, nDCType, 0);
                                        }

                                        damageWearItem(WLG_WEAPON, SYS_WEAPONDURALOSSODDS);

                                        if(m_nextStrike){
                                            m_nextStrike = false;
                                        }
                                        else{
                                            m_nextStrike = (mathf::rand<int>(0, 2) == 0);
                                        }

                                        if(m_nextStrike){
                                            reportNextStrike();
                                        }
                                        co_return;
                                    }
                                default:
                                    {
                                        co_return;
                                    }
                            }
                            co_return;
                        }
                    case 1:
                        {
                            if(co_await requestMove(nX0, nY0, SYS_MAXSPEED, false, false)){
                                co_await onCMActionAttack(stCMA);
                            }
                            else{
                                reportStand();
                            }
                            co_return;
                        }
                    default:
                        {
                            co_return;
                        }
                }
                co_return;
            }
        default:
            {
                co_return;
            }
    }
}

corof::awaitable<> Player::onCMActionSpinKick(CMAction cmA)
{
    fflassert(cmA.action.type == ACTION_SPINKICK);
    dispatchAction(cmA.action);
    return {};
}

corof::awaitable<> Player::onCMActionPickUp(CMAction cmA)
{
    fflassert(cmA.action.type == ACTION_PICKUP);
    dispatchAction(cmA.action);
    return {};
}

corof::awaitable<> Player::onCMActionSpell(CMAction cmA)
{
    fflassert(cmA.action.type == ACTION_SPELL);
    const auto magicID = cmA.action.extParam.spell.magicID;

    dispatchAction(cmA.action);
    const auto node = getCombatNode(m_sdItemStorage.wear, m_sdLearnedMagicList, UID(), level());

    switch(magicID){
        case DBCOM_MAGICID(u8"治愈术"):
        case DBCOM_MAGICID(u8"施毒术"):
        case DBCOM_MAGICID(u8"幽灵盾"):
        case DBCOM_MAGICID(u8"神圣战甲术"):
            {
                const auto buffID = DBCOM_BUFFID(DBCOM_MAGICRECORD(magicID).name);
                const auto &br = DBCOM_BUFFRECORD(buffID);

                fflassert(buffID);
                fflassert(br);

                if(cmA.action.aimUID){
                    switch(uidf::getUIDType(cmA.action.aimUID)){
                        case UID_MON:
                        case UID_PLY:
                            {
                                if(br.favor == 0){
                                    sendBuff(cmA.action.aimUID, 0, buffID);
                                }
                                else{
                                    switch(const auto friendType = co_await checkFriend(cmA.action.aimUID); friendType){
                                        case FT_FRIEND:
                                            {
                                                if(br.favor >= 0){
                                                    sendBuff(cmA.action.aimUID, 0, buffID);
                                                }
                                                co_return;
                                            }
                                        case FT_ENEMY:
                                            {
                                                if(br.favor <= 0 && co_await canDamageTarget(cmA.action.aimUID)){
                                                    sendBuff(cmA.action.aimUID, 0, buffID);
                                                }
                                                co_return;
                                            }
                                        case FT_NEUTRAL:
                                            {
                                                sendBuff(cmA.action.aimUID, 0, buffID);
                                                co_return;
                                            }
                                        default:
                                            {
                                                co_return;
                                            }
                                    }
                                }
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                }
                else if(br.favor >= 0){
                    addBuff(UID(), 0, buffID);
                }
                break;
            }
        case DBCOM_MAGICID(u8"火球术"):
        case DBCOM_MAGICID(u8"大火球"):
        case DBCOM_MAGICID(u8"灵魂火符"):
        case DBCOM_MAGICID(u8"冰月神掌"):
        case DBCOM_MAGICID(u8"冰月震天"):
            {
                // 灵魂火符 doesn't need to send back the CASTMAGIC message
                // the ACTION_SPELL creates the magic

                if(cmA.action.aimUID){
                    const auto coLocOpt = co_await getCOLocation(cmA.action.aimUID);

                    if(!coLocOpt.has_value()){
                        co_return;
                    }

                    const auto &coLoc = coLocOpt.value();
                    const auto ld = mathf::LDistance<float>(coLoc.x, coLoc.y, cmA.action.x, cmA.action.y);
                    const auto delay = ld * 100;

                    addDelay(delay, [cmA, this](bool) -> corof::awaitable<>
                    {
                        if(!co_await canDamageTarget(cmA.action.aimUID)){
                            co_return;
                        }
                        markAggression(co_await queryPlayerController(cmA.action.aimUID));
                        dispatchAttackDamage(cmA.action.aimUID, cmA.action.extParam.spell.magicID, 0);
                        co_return;
                    });
                }
                break;
            }
        case DBCOM_MAGICID(u8"雷电术"):
            {
                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID     = UID();
                smFM.mapUID  = mapUID();
                smFM.Magic   = magicID;
                smFM.Speed   = MagicSpeed();
                smFM.X       = cmA.action.x;
                smFM.Y       = cmA.action.y;
                smFM.AimUID  = cmA.action.aimUID;

                addDelay(1400, [this, smFM](bool)
                {
                    dispatchNetPackage(true, SM_CASTMAGIC, smFM);
                    addDelay(300, [smFM, this](bool) -> corof::awaitable<>
                    {
                        if(!co_await canDamageTarget(smFM.AimUID)){
                            co_return;
                        }
                        markAggression(co_await queryPlayerController(smFM.AimUID));
                        dispatchAttackDamage(smFM.AimUID, DBCOM_MAGICID(u8"雷电术"), 0);
                        co_return;
                    });
                });
                break;
            }
        case DBCOM_MAGICID(u8"魔法盾"):
        case DBCOM_MAGICID(u8"阴阳法环"):
            {
                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID   = UID();
                smFM.Magic = magicID;
                smFM.Speed = MagicSpeed();

                addDelay(800, [this, smFM](bool)
                {
                    dispatchNetPackage(true, SM_CASTMAGIC, smFM);
                    addDelay(10000, [this](bool)
                    {
                        SMBuff smB;
                        std::memset(&smB, 0, sizeof(smB));

                        smB.uid   = UID();
                        smB.type  = BFT_SHIELD;
                        smB.state = BFS_OFF;
                        dispatchNetPackage(true, SM_BUFF, smB);
                    });
                });
                break;
            }
        case DBCOM_MAGICID(u8"召唤骷髅"):
        case DBCOM_MAGICID(u8"超强召唤骷髅"):
            {
                const auto [nFrontX, nFrontY] = pathf::getFrontGLoc(X(), Y(), Direction(), 2);

                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID    = UID();
                smFM.mapUID = mapUID();
                smFM.Magic  = magicID;
                smFM.Speed  = MagicSpeed();
                smFM.AimX   = nFrontX;
                smFM.AimY   = nFrontY;

                addDelay(600, [magicID, smFM, thisptr = this](this auto, bool) -> corof::awaitable<>
                {
                    for(int i = 0; i < g_serverArgParser->sharedConfig().summonCount; ++i){
                        if(to_u32(magicID) == DBCOM_MAGICID(u8"召唤骷髅")){
                            co_await thisptr->addMonster(DBCOM_MONSTERID(u8"变异骷髅"), smFM.AimX, smFM.AimY, false);
                        }
                        else{
                            co_await thisptr->addMonster(DBCOM_MONSTERID(u8"超强骷髅"), smFM.AimX, smFM.AimY, false);
                        }
                    }

                    // addMonster will send ACTION_SPAWN to client
                    // client then use it to play the magic for 召唤骷髅, we don't send magic message here
                });
                break;
            }
        case DBCOM_MAGICID(u8"召唤神兽"):
            {
                const auto [nFrontX, nFrontY] = pathf::getFrontGLoc(X(), Y(), Direction(), 2);

                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID   = UID();
                smFM.mapUID = mapUID();
                smFM.Magic = magicID;
                smFM.Speed = MagicSpeed();
                smFM.AimX  = nFrontX;
                smFM.AimY  = nFrontY;

                addDelay(1000, [smFM, thisptr = this](this auto, bool) -> corof::awaitable<>
                {
                    for(int i = 0; i < g_serverArgParser->sharedConfig().summonCount; ++i){
                        co_await thisptr->addMonster(DBCOM_MONSTERID(u8"神兽"), smFM.AimX, smFM.AimY, false);
                    }
                });
                break;
            }
        case DBCOM_MAGICID(u8"火墙"):
            {
                addDelay(550, [this, cmA, node](bool)
                {
                    AMCastFireWall amCFW;
                    std::memset(&amCFW, 0, sizeof(amCFW));

                    amCFW.minDC = node.mc[0];
                    amCFW.maxDC = node.mc[1];
                    amCFW.mcHit = node.mcHit;

                    amCFW.duration = 20 * 1000;
                    amCFW.dps      = 2;

                    // not 3x3
                    // fire wall takes grids as a cross
                    //
                    // +---+---+---+
                    // |   | v |   |
                    // +---+---+---+
                    // | v | v | v |
                    // +---+---+---+
                    // |   | v |   |
                    // +---+---+---+

                    for(const int dir: {DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT}){
                        if(dir == DIR_NONE){
                            amCFW.x = cmA.action.aimX;
                            amCFW.y = cmA.action.aimY;
                        }
                        else{
                            std::tie(amCFW.x, amCFW.y) = pathf::getFrontGLoc(cmA.action.aimX, cmA.action.aimY, dir, 1);
                        }

                        if(mapBin()->groundValid(amCFW.x, amCFW.y)){
                            m_actorPod->post(mapUID(), {AM_CASTFIREWALL, amCFW});
                        }
                    }
                });
                break;
            }
        case DBCOM_MAGICID(u8"地狱火"):
        case DBCOM_MAGICID(u8"冰沙掌"):
        case DBCOM_MAGICID(u8"疾光电影"):
            {
                if(const auto dirIndex = pathf::getDir8(cmA.action.aimX - cmA.action.x, cmA.action.aimY - cmA.action.y); (dirIndex >= 0) && pathf::dirValid(dirIndex + DIR_BEGIN)){
                    m_direction = dirIndex + DIR_BEGIN;
                }

                std::set<std::tuple<int, int>> pathGridList;
                switch(Direction()){
                    case DIR_UP:
                    case DIR_DOWN:
                    case DIR_LEFT:
                    case DIR_RIGHT:
                        {
                            for(const auto distance: {1, 2, 3, 4, 5, 6, 7, 8}){
                                const auto [pathGX, pathGY] = pathf::getFrontGLoc(X(), Y(), Direction(), distance);
                                pathGridList.insert({pathGX, pathGY});

                                if(distance > 3){
                                    const auto [sgnDX, sgnDY] = pathf::getFrontGLoc(0, 0, Direction(), 1);
                                    pathGridList.insert({pathGX + sgnDY, pathGY + sgnDX}); // switch sgnDX and sgnDY and plus/minus
                                    pathGridList.insert({pathGX - sgnDY, pathGY - sgnDX});
                                }
                            }
                            break;
                        }
                    case DIR_UPLEFT:
                    case DIR_UPRIGHT:
                    case DIR_DOWNLEFT:
                    case DIR_DOWNRIGHT:
                        {
                            for(const auto distance: {1, 2, 3, 4, 5, 6, 7, 8}){
                                const auto [pathGX, pathGY] = pathf::getFrontGLoc(X(), Y(), Direction(), distance);
                                pathGridList.insert({pathGX, pathGY});

                                const auto [sgnDX, sgnDY] = pathf::getFrontGLoc(0, 0, Direction(), 1);
                                pathGridList.insert({pathGX + sgnDX, pathGY        });
                                pathGridList.insert({pathGX        , pathGY + sgnDY});
                            }
                            break;
                        }
                    default:
                        {
                            throw fflreach();
                        }
                }

                AMStrikeFixedLocDamage amSFLD;
                std::memset(&amSFLD, 0, sizeof(amSFLD));

                for(const auto &[pathGX, pathGY]: pathGridList){
                    if(mapBin()->groundValid(pathGX, pathGY)){
                        amSFLD.x = pathGX;
                        amSFLD.y = pathGY;
                        amSFLD.damage = getAttackDamage(magicID, 0);
                        addDelay(550 + mathf::CDistance(X(), Y(), amSFLD.x, amSFLD.y) * 100, [amSFLD, castMapID = mapID(), this](bool)
                        {
                            if(castMapID == mapID()){
                                m_actorPod->post(mapUID(), {AM_STRIKEFIXEDLOCDAMAGE, amSFLD});
                                if(g_serverArgParser->sharedConfig().showStrikeGrid){
                                    SMStrikeGrid smSG;
                                    std::memset(&smSG, 0, sizeof(smSG));

                                    smSG.x = amSFLD.x;
                                    smSG.y = amSFLD.y;
                                    dispatchNetPackage(true, SM_STRIKEGRID, smSG);
                                }
                            }
                        });
                    }
                }
                break;
            }
        default:
            {
                break;
            }
    }
}

void Player::gainExp(int addedExp)
{
    if(addedExp <= 0){
        return;
    }

    const auto oldLevel = level();
    const auto oldMaxHP = maxHP();
    const auto oldMaxMP = maxMP();

    m_exp += addedExp;
    m_luaRunner->spawn(m_threadKey++, str_printf("_RSVD_NAME_trigger(SYS_ON_GAINEXP, %d)", addedExp));

    const auto addedMaxHP = std::max<int>(maxHP() - oldMaxHP, 0);
    const auto addedMaxMP = std::max<int>(maxMP() - oldMaxMP, 0);

    dbUpdateExp();
    postExp();

    if(level() > oldLevel){
        m_luaRunner->spawn(m_threadKey++, str_printf("_RSVD_NAME_trigger(SYS_ON_LEVELUP, %d, %d)", to_d(oldLevel), to_d(level())));
    }

    if(addedMaxHP > 0 || addedMaxMP > 0){
        updateHealth(0, 0, addedMaxHP, addedMaxMP);
    }
}

bool Player::CanPickUp(uint32_t, uint32_t)
{
    return true;
}

void Player::reportGold()
{
    SMGold smG;
    std::memset(&smG, 0, sizeof(smG));
    smG.gold = gold();
    postNetMessage(SM_GOLD, smG);
}

void Player::reportUpdateItem(const SDItem &item)
{
    postNetMessage(SM_UPDATEITEM, cerealf::serialize(SDUpdateItem
    {
        .item = item,
    }));
}

void Player::reportRemoveItem(uint32_t itemID, uint32_t seqID, size_t count)
{
    SMRemoveItem smRI;
    std::memset(&smRI, 0, sizeof(smRI));

    smRI.itemID = itemID;
    smRI. seqID =  seqID;
    smRI. count =  count;
    postNetMessage(SM_REMOVEITEM, smRI);
}

void Player::reportSecuredItemList()
{
    postNetMessage(SM_SHOWSECUREDITEMLIST, cerealf::serialize(SDShowSecuredItemList
    {
        .itemList = dbLoadSecuredItemList(),
    }));
}

corof::awaitable<> Player::reportTeamMemberList()
{
    const auto sdTMLOpt = co_await pullTeamMemberList();
    if(sdTMLOpt.has_value()){
        postNetMessage(SM_TEAMMEMBERLIST, cerealf::serialize(sdTMLOpt.value()));
    }
}

corof::awaitable<int> Player::checkFriend(uint64_t targetUID)
{
    // this function means:
    // this player says: how I fell about targetUID

    fflassert(targetUID);
    fflassert(targetUID != UID());

    switch(uidf::getUIDType(targetUID)){
        case UID_NPC:
            {
                co_return FT_NEUTRAL;
            }
        case UID_PLY:
            {
                co_return isOffender(targetUID) ? FT_ENEMY : FT_NEUTRAL;
            }
        case UID_MON:
            {
                if(const auto finalMasterUID = co_await queryFinalMaster(targetUID)){
                    switch(uidf::getUIDType(finalMasterUID)){
                        case UID_PLY:
                            {
                                co_return isOffender(finalMasterUID) ? FT_ENEMY : FT_NEUTRAL;
                            }
                        case UID_MON:
                            {
                                co_return FT_ENEMY;
                            }
                        default:
                            {
                                throw fflvalue(uidf::getUIDString(finalMasterUID));
                            }
                    }
                }
                else{
                    co_return FT_ERROR;
                }
            }
        default:
            {
                throw fflvalue(uidf::getUIDString(targetUID));
            }
    }
}

void Player::requestDie()
{
    goDie();
}

void Player::requestKillPets()
{
    for(auto uid: m_slaveList){
        m_actorPod->post(uid, {AM_MASTERKILL});
    }
    m_slaveList.clear();
}

void Player::postOnlineOK()
{
    SMOnlineOK smOOK;
    std::memset(&smOOK, 0, sizeof(smOOK));

    smOOK.uid = UID();
    smOOK.name.assign(m_name);
    smOOK.gender = gender();
    smOOK.job = job();
    smOOK.mapUID = mapUID();
    smOOK.action = makeActionStand();

    postNetMessage(SM_ONLINEOK, smOOK);
    postNetMessage(SM_STARTGAMESCENE, cerealf::serialize(SDStartGameScene
    {
        .uid = UID(),
        .mapUID = mapUID(),

        .x = X(),
        .y = Y(),
        .direction = Direction(),

        .desp
        {
            .wear = m_sdItemStorage.wear,
            .hair = m_hair,
            .hairColor = m_hairColor,
        },

        .name = m_name,
        .nameColor = nameColor(),
    }));

    postExp();
    postNetMessage(SM_HEALTH,           cerealf::serialize(m_sdHealth));
    postNetMessage(SM_INVENTORY,        cerealf::serialize(m_sdItemStorage.inventory));
    postNetMessage(SM_BELT,             cerealf::serialize(m_sdItemStorage.belt));
    postNetMessage(SM_LEARNEDMAGICLIST, cerealf::serialize(m_sdLearnedMagicList));
    postNetMessage(SM_PLAYERCONFIG,     cerealf::serialize(m_sdPlayerConfig));
    postNetMessage(SM_FRIENDLIST,       cerealf::serialize(m_sdFriendList));

    std::vector<uint64_t> friendIDList
    {
        SDChatPeerID(CPR_SPECIAL, SYS_CHATDBID_SYSTEM).asU64(),
        SDChatPeerID(CPR_PLAYER , dbid()             ).asU64(),
    };

    std::for_each(m_sdFriendList.begin(), m_sdFriendList.end(), [&friendIDList](const auto &peer)
    {
        friendIDList.push_back(peer.cpid().asU64());
    });

    if(!friendIDList.empty()){
        postNetMessage(SM_CHATMESSAGELIST, cerealf::serialize(dbRetrieveLatestChatMessage(dbid(), friendIDList, 1, true, true)));
    }

    for(int wltype = WLG_BEGIN; wltype < WLG_END; ++wltype){
        if(const auto &item = m_sdItemStorage.wear.getWLItem(wltype)){
            if(const auto buffIDOpt = item.getExtAttr<SDItem::EA_BUFFID_t>(); buffIDOpt.has_value() && buffIDOpt.value()){
                if(const auto pbuff = addBuff(UID(), 0, buffIDOpt.value())){
                    addWLOffTrigger(wltype, [buffSeq = pbuff->buffSeq(), this]()
                    {
                        removeBuff(buffSeq, true);
                    });
                }
            }
        }
    }
}

bool Player::hasInventoryItem(uint32_t itemID, uint32_t seqID, size_t count) const
{
    return m_sdItemStorage.inventory.has(itemID, seqID) >= count;
}

const SDItem &Player::addInventoryItem(SDItem item, bool keepSeqID)
{
    const auto itemID = item.itemID;
    const auto &addedItem = m_sdItemStorage.inventory.add(std::move(item), keepSeqID);
    dbUpdateInventoryItem(dbid(), addedItem);

    m_luaRunner->spawn(m_threadKey++, str_printf("_RSVD_NAME_trigger(SYS_ON_GAINITEM, %llu)", to_llu(itemID)));
    reportUpdateItem(addedItem);
    return addedItem;
}

size_t Player::removeInventoryItem(const SDItem &item)
{
    return removeInventoryItem(item.itemID, item.seqID);
}

size_t Player::removeInventoryItem(uint32_t itemID, uint32_t seqID)
{
    fflassert(DBCOM_ITEMRECORD(itemID));
    fflassert(seqID > 0);
    return removeInventoryItem(itemID, seqID, SIZE_MAX);
}

size_t Player::removeInventoryItem(uint32_t itemID, uint32_t seqID, size_t count)
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);

    fflassert(ir);
    fflassert(count > 0);
    fflassert(!ir.isGold());

    size_t doneCount = 0;
    while(doneCount < count){
        const auto [removedCount, removedSeqID, itemPtr] = m_sdItemStorage.inventory.remove(itemID, seqID, count - doneCount, false);
        if(!removedCount){
            break;
        }

        if(itemPtr){
            dbUpdateInventoryItem(dbid(), *itemPtr);
        }
        else{
            dbRemoveInventoryItem(dbid(), itemID, removedSeqID);
        }

        doneCount += removedCount;
        reportRemoveItem(itemID, removedSeqID, removedCount);
    }
    return doneCount;
}

const SDItem *Player::findInventoryItem(uint32_t itemID, uint32_t seqID) const
{
    fflassert(DBCOM_ITEMRECORD(itemID));
    return m_sdItemStorage.inventory.find(itemID, seqID);
}

SDItem *Player::findInventoryItem(uint32_t itemID, uint32_t seqID)
{
    fflassert(DBCOM_ITEMRECORD(itemID));
    return m_sdItemStorage.inventory.find(itemID, seqID);
}

void Player::secureItem(uint32_t itemID, uint32_t seqID)
{
    fflassert(findInventoryItem(itemID, seqID));
    dbSecureItem(itemID, seqID);
    removeInventoryItem(itemID, seqID);
}

void Player::removeSecuredItem(uint32_t itemID, uint32_t seqID)
{
    addInventoryItem(dbRetrieveSecuredItem(itemID, seqID), false);

    SMRemoveSecuredItem smRSI;
    std::memset(&smRSI, 0, sizeof(smRSI));

    smRSI.itemID = itemID;
    smRSI. seqID =  seqID;
    postNetMessage(SM_REMOVESECUREDITEM, smRSI);
}

void Player::setGold(size_t gold)
{
    m_sdItemStorage.gold = gold;
    dbUpdateGold(dbid(), m_sdItemStorage.gold);
    reportGold();
}

void Player::addGold(size_t extraGold)
{
    setGold(gold() + extraGold);
}

bool Player::updateHealth(int addHP, int addMP, int addMaxHP, int addMaxMP)
{
    if(BattleObject::updateHealth(addHP, addMP, addMaxHP, addMaxMP)){
        dbUpdateHealth();
        postNetMessage(SM_HEALTH, cerealf::serialize(m_sdHealth));
        return true;
    }
    return false;
}

bool Player::setHealth(std::optional<int> hp, std::optional<int> mp, std::optional<int> maxHP, std::optional<int> maxMP)
{
    if(BattleObject::setHealth(hp, mp, maxHP, maxMP)){
        dbUpdateHealth();
        postNetMessage(SM_HEALTH, cerealf::serialize(m_sdHealth));
        return true;
    }
    return false;
}

void Player::setWLItem(int wltype, SDItem item)
{
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflpanic("bad wltype: {}", wltype);
    }

    m_sdItemStorage.wear.setWLItem(wltype, item);
    const auto sdEquipWearBuf = cerealf::serialize(SDEquipWear
    {
        .uid = UID(),
        .wltype = wltype,
        .item = item,
    });

    foreachInViewCO([sdEquipWearBuf, this](const COLocation &coLoc)
    {
        if(uidf::getUIDType(coLoc.uid) == UID_PLY){
            forwardNetPackage(coLoc.uid, SM_EQUIPWEAR, sdEquipWearBuf);
        }
    });
}

bool Player::damageWearItem(int wltype, int odds)
{
    fflassert(wltype >= WLG_BEGIN, wltype);
    fflassert(wltype <  WLG_END  , wltype);
    fflassert(odds > 0, odds);

    const auto item = m_sdItemStorage.wear.getWLItem(wltype);
    if(item.duration[1] == 0){
        return false;
    }

    if(mathf::rand<int>(1, odds) != 1){
        return false;
    }

    if(item.duration[0] > 0){
        auto damagedItem = item;
        damagedItem.duration[0]--;

        m_sdItemStorage.wear.setWLItem(wltype, damagedItem);
        dbUpdateWearItem(wltype, damagedItem);

        SMWearItemDuration smWID;
        std::memset(&smWID, 0, sizeof(smWID));

        smWID.wltype = to_u32(wltype);
        smWID.duration[0] = to_u32(damagedItem.duration[0]);
        smWID.duration[1] = to_u32(damagedItem.duration[1]);

        postNetMessage(SM_WEARITEMDURATION, smWID);
        return false;
    }

    // durability has been exhausted, this loss destroys the item permanently
    // don't put it back into the inventory, it's gone

    setWLItem(wltype, {});
    dbRemoveWearItem(wltype);

    postNetMessage(SM_EQUIPWEAR, cerealf::serialize(SDEquipWear
    {
        .uid = UID(),
        .wltype = wltype,
    }));

    postNetMessage(SM_TEXT, str_printf(u8"%s因持久耗尽而彻底损坏", to_cstr(DBCOM_ITEMRECORD(item.itemID).name)));

    if(auto cbp = m_onWLOff.find(wltype); cbp != m_onWLOff.end()){
        fflassert(cbp->second);
        cbp->second();
        m_onWLOff.erase(cbp);
    }
    return true;
}

void Player::damageDefendWearItem()
{
    std::vector<int> wltypeList;
    for(int wltype = WLG_BEGIN; wltype < WLG_END; ++wltype){
        if(wltype != WLG_WEAPON && m_sdItemStorage.wear.getWLItem(wltype).duration[1] > 0){
            wltypeList.push_back(wltype);
        }
    }

    if(!wltypeList.empty()){
        damageWearItem(wltypeList.at(mathf::rand<size_t>(0, wltypeList.size() - 1)), SYS_ARMORDURALOSSODDS);
    }
}

// may this attack be carried into damage calculation against targetUID
//
// player can always swing at anyone and the attack gfx always plays, this only decides
// whether the swing resolves into damage at all
//
// true does not promise damage > 0, the target may still soak all of it with armor
corof::awaitable<bool> Player::canDamageTarget(uint64_t targetUID)
{
    fflassert(targetUID);

    if(targetUID == UID()){
        co_return false;
    }

    // a safe zone is absolute: it outranks retaliation and every attack mode
    //
    // nothing standing on a safe grid can be damaged, and nothing can deal damage out of one,
    // otherwise a town would only be safe against players who never got provoked

    if(inSafeZone()){
        co_return false;
    }

    if(const auto coLocPtr = getInViewCOPtr(targetUID); coLocPtr && isSafeGrid(coLocPtr->x, coLocPtr->y)){
        co_return false;
    }

    // a monster answers to the player controlling it, so a pet inherits its master's
    // protection, while anything with no player behind it is always free to hit

    const auto playerUID = co_await queryPlayerController(targetUID);
    if(!playerUID){
        co_return true;
    }

    if(playerUID == UID()){
        co_return false; // my own pet
    }

    // whoever hit me first can always be hit back, no matter the attack mode

    if(isOffender(playerUID)){
        co_return true;
    }

    switch(attackMode()){
        case ATKMODE_ALL:
            {
                co_return true;
            }
        case ATKMODE_GROUP:
        case ATKMODE_GUILD:
            {
                // there is no guild yet, so 行会 behaves as 编队 until one exists
                // both spare the team, ATKMODE_GUILD will additionally spare guild members
                co_return std::find(m_teamMemberList.begin(), m_teamMemberList.end(), playerUID) == m_teamMemberList.end();
            }
        default:
            {
                co_return false;
            }
    }
}

void Player::markAggression(uint64_t targetUID)
{
    // takes the result of queryPlayerController() as is, zero means nothing player controlled
    // got hit and there is nobody to answer for, hitting my own pet is nobody's fault either

    if(!targetUID || targetUID == UID()){
        return;
    }

    fflassert(uidf::isPlayer(targetUID), uidf::getUIDString(targetUID));

    // keep refreshing while I keep swinging, a long fight must not let the mark expire
    // and turn my own murder into self defense

    if(auto p = m_aggressionList.find(targetUID); p != m_aggressionList.end()){
        p->second = hres_tstamp().to_sec();
    }
    else if(!isOffender(targetUID)){
        // it never hit me, so this strike is mine to answer for
        m_aggressionList[targetUID] = hres_tstamp().to_sec();
    }
}

bool Player::hasAggression(uint64_t targetUID) const
{
    if(const auto p = m_aggressionList.find(targetUID); p != m_aggressionList.end()){
        return hres_tstamp().to_sec() < p->second + SYS_PKAGGRESSIONTIME;
    }
    return false;
}

void Player::addPKPoint(int added)
{
    fflassert(added != 0, added);

    const auto wasRedName = redName();
    m_pkPoint = std::max<int>(0, m_pkPoint + added);

    dbUpdatePKPoint();
    if(wasRedName != redName()){
        // name color is derived from the pk point, tell everyone who can see me
        reportName();
        postNetMessage(SM_TEXT, str_printf(u8"%s", redName() ? u8"你已经变成红名，城里的商人不会再和你交易" : u8"你的红名已经洗清"));
    }
}

void Player::reportName()
{
    dispatchNetPackage(true, SM_PLAYERNAME, cerealf::serialize(SDPlayerName
    {
        .uid = UID(),
        .name = name(),
        .nameColor = nameColor(),
    }));
}

bool Player::repairInventoryItem(uint32_t itemID, uint32_t seqID, bool special)
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);
    if(!ir || ir.isGold()){
        return false;
    }

    const auto item = findInventoryItem(itemID, seqID);
    if(!item || item->duration[0] >= item->duration[1]){
        return false;
    }

    if(!special){
        // normal repair permanently wears the item out by 1/SYS_DURALOSSRATE of the restored
        // durability, the remainder is taken by chance so that repairing a slightly damaged
        // item still risks the max durability, this is why 特殊修理 exists

        const auto restored = item->duration[1] - item->duration[0];
        const auto lostMax = restored / SYS_DURALOSSRATE + (to_uz(mathf::rand<int>(1, SYS_DURALOSSRATE)) <= restored % SYS_DURALOSSRATE ? 1 : 0);

        item->duration[1] = std::max<size_t>(1, item->duration[1] - lostMax);
    }

    item->duration[0] = item->duration[1];

    dbUpdateInventoryItem(dbid(), *item);
    reportUpdateItem(*item);
    return true;
}

void Player::postExp()
{
    SMExp smE;
    std::memset(&smE, 0, sizeof(smE));
    smE.exp = exp();
    postNetMessage(SM_EXP, smE);
}

bool Player::canWear(uint32_t itemID, int wltype) const
{
    fflassert(itemID, itemID);
    fflassert(wltype >= WLG_BEGIN, wltype);
    fflassert(wltype <  WLG_END  , wltype);

    const auto &ir = DBCOM_ITEMRECORD(itemID);
    if(!ir){
        return false;
    }

    if(!ir.wearable(wltype)){
        return false;
    }

    if(wltype == WLG_DRESS && (!ir.clothGender().has_value() || ir.clothGender().value() != gender())){
        return false;
    }

    // TODO
    // check item requirement

    return true;
}

std::vector<std::string> Player::parseRemoteCall(const char *query)
{
    fflassert(str_haschar(query));

    const char *beginPtr = query;
    const char *endPtr   = query + std::strlen(query);

    std::vector<std::string> result;
    while(true){
        beginPtr = std::find_if_not(beginPtr, endPtr, [](char chByte)
        {
            return chByte == ' ';
        });

        if(beginPtr == endPtr){
            break;
        }

        const char *donePtr = std::find(beginPtr, endPtr, ' ');
        result.emplace_back(beginPtr, donePtr);
        beginPtr = donePtr;
    }
    return result;
}

void Player::afterChangeGLoc()
{
    for(const auto uid: m_slaveList){
        dispatchAction(uid, makeActionStand());
    }

    if(m_teamLeader){
        for(const auto uid: m_teamMemberList){
            if(uid != UID()){
                dispatchAction(uid, makeActionStand());
            }
        }
    }
}

int Player::maxHP() const
{
    const int maxHPTaoist  = 100 + level() *  50;
    const int maxHPWarrior = 300 + level() * 100;
    const int maxHPWizard  =  50 + level() *  20;

    int result = 0;
    if(job() & JOB_WARRIOR) result = std::max<int>(result, maxHPWarrior);
    if(job() & JOB_TAOIST ) result = std::max<int>(result, maxHPTaoist );
    if(job() & JOB_WIZARD ) result = std::max<int>(result, maxHPWizard );
    return result;
}

int Player::maxMP() const
{
    const int maxMPTaoist  = 200 + level() *  50;
    const int maxMPWarrior = 100 + level() *  10;
    const int maxMPWizard  = 500 + level() * 200;

    int result = 0;
    if(job() & JOB_WARRIOR) result = std::max<int>(result, maxMPWarrior);
    if(job() & JOB_TAOIST ) result = std::max<int>(result, maxMPTaoist );
    if(job() & JOB_WIZARD ) result = std::max<int>(result, maxMPWizard );
    return result;
}

bool Player::consumeBook(uint32_t itemID)
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);
    fflassert(ir);
    fflassert(ir.isBook());

    // a plain 技能书 is raw material, a 武功教头 has to copy it out first
    if(!ir.isMagicBook()){
        postNetMessage(SM_TEXT, str_printf(u8"%s只是一本旧书，要找武功教头誊抄成秘籍才能修炼", to_cstr(ir.name)));
        return false;
    }

    const auto magicID = DBCOM_MAGICID(ir.book.magic);
    const auto &mr = DBCOM_MAGICRECORD(magicID);

    fflassert(magicID);
    fflassert(mr);

    if(m_sdLearnedMagicList.has(magicID)){
        postNetMessage(SM_TEXT, str_printf(u8"无法学习%s，因为你已掌握此技能", to_cstr(mr.name)));
        return false;
    }

    if(!g_serverArgParser->sharedConfig().disableLearnMagicCheckJob){
        bool hasJob = false;
        for(const auto jobstr: jobf::jobName(job())){
            if(to_u8sv(jobstr) == mr.req.job){
                hasJob = true;
            }
        }

        if(!hasJob){
            postNetMessage(SM_TEXT, str_printf(u8"无法学习%s，因为此项技能需要职业为%s", to_cstr(mr.name), to_cstr(mr.req.job)));
            return false;
        }
    }

    if(to_d(level()) < mr.req.level[0] && !g_serverArgParser->sharedConfig().disableLearnMagicCheckLevel){
        postNetMessage(SM_TEXT, str_printf(u8"无法学习%s，因为你尚未到达%d级", to_cstr(mr.name), mr.req.level[0]));
        return false;
    }

    if(str_haschar(mr.req.prior) && !g_serverArgParser->sharedConfig().disableLearnMagicCheckPrior){
        const auto priorMagicID = DBCOM_MAGICID(mr.req.prior);
        const auto &priorMR = DBCOM_MAGICRECORD(priorMagicID);

        fflassert(priorMagicID);
        fflassert(priorMR);

        if(!m_sdLearnedMagicList.has(priorMagicID)){
            postNetMessage(SM_TEXT, str_printf(u8"无法学习%s，因为你尚未学习前置魔法%s", to_cstr(mr.name), to_cstr(priorMR.name)));
            return false;
        }
    }

    m_sdLearnedMagicList.magicList.push_back(SDLearnedMagic
    {
        .magicID = magicID,
    });

    dbLearnMagic(magicID);
    postNetMessage(SM_TEXT, str_printf(u8"恭喜掌握%s", to_cstr(mr.name)));
    postNetMessage(SM_LEARNEDMAGICLIST, cerealf::serialize(m_sdLearnedMagicList));
    return true;
}

bool Player::consumePotion(uint32_t itemID)
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);
    fflassert(ir);
    fflassert(ir.isPotion());

    if(addBuff(UID(), 0, DBCOM_BUFFID(ir.name))){
        return true;
    }
    return false;
}

corof::awaitable<std::optional<SDTeamMemberList>> Player::pullTeamMemberList()
{
    if(!m_teamLeader){
        co_return SDTeamMemberList{};
    }

    if(m_teamLeader != UID()){
        switch(const auto rmpk = co_await m_actorPod->send(m_teamLeader, AM_QUERYTEAMMEMBERLIST); rmpk.type()){
            case AM_TEAMMEMBERLIST:
                {
                    const auto sdTML = rmpk.deserialize<SDTeamMemberList>();
                    fflassert(sdTML.hasMember(UID())); // keep this function read only
                    co_return sdTML;
                }
            default:
                {
                    co_return std::nullopt;
                }
        }
    }

    SDTeamMemberList sdTML;

    sdTML.teamLeader = m_teamLeader;
    sdTML.memberList.resize(m_teamMemberList.size());

    for(size_t i = 0; i < m_teamMemberList.size(); ++i){
        if(m_teamMemberList.at(i) == UID()){
            sdTML.memberList[i] = SDTeamPlayer
            {
                .uid = UID(),
                .level = level(),
                .name = name(),
            };
        }
        else{
            switch(const auto mpk = co_await m_actorPod->send(m_teamMemberList.at(i), AM_QUERYTEAMPLAYER); mpk.type()){
                case AM_TEAMPLAYER:
                    {
                        sdTML.memberList.at(i) = mpk.deserialize<SDTeamPlayer>();
                        break;
                    }
                default:
                    {
                        throw fflvalue(mpk.str());
                    }
            }
        }
    }

    co_return sdTML;
}

corof::awaitable<bool> Player::followTeamLeader()
{
    if(!m_teamLeader){
        co_return false;
    }

    const auto coLocOpt = co_await getCOLocation(m_teamLeader);
    if(!coLocOpt.has_value()){
        co_return false;
    }

    const auto fnRequestMove = [thisptr = this](this auto, int dstX, int dstY) -> corof::awaitable<bool>
    {
        BattleObject::BOPathFinder finder(thisptr, 1);
        if(!finder.search(thisptr->X(), thisptr->Y(), thisptr->Direction(), dstX, dstY).hasPath()){
            co_return false;
        }

        const auto oldX = thisptr->X();
        const auto oldY = thisptr->Y();

        const auto nextGLoc = finder.getPathNode().at(1);
        const auto doneMove = co_await thisptr->requestMove(nextGLoc.X, nextGLoc.Y, thisptr->moveSpeed(), false, false);

        if(doneMove){
            thisptr->reportAction(thisptr->UID(), thisptr->mapUID(), ActionMove
            {
                .speed = SYS_DEFSPEED,
                .x = oldX,
                .y = oldY,
                .aimX = thisptr->X(),
                .aimY = thisptr->Y(),
            });
        }

        co_return doneMove;
    };

    if(const auto &coLoc = coLocOpt.value(); coLoc.mapUID != mapUID()){
        const auto [backX, backY] = pathf::getBackGLoc(coLoc.x, coLoc.y, coLoc.direction, 1);
        co_return co_await requestMapSwitch(coLoc.mapUID, backX, backY, false);
    }
    else if(const auto cdist = mathf::CDistance<double>(coLoc.x, coLoc.y, X(), Y()); cdist <= 1){
        const auto [backX, backY] = pathf::getBackGLoc(coLoc.x, coLoc.y, coLoc.direction, 1);
        switch(mathf::LDistance2<int>(backX, backY, X(), Y())){
            case 0:
                {
                    if(Direction() != coLoc.direction){
                        m_direction = coLoc.direction;
                        dispatchAction(makeActionStand());
                    }
                    co_return true;
                }
            default:
                {
                    co_return co_await fnRequestMove(backX, backY);
                }
        }
    }
    else if(cdist < 10){
        co_return co_await fnRequestMove(coLoc.x, coLoc.y);
    }
    else{
        const auto [backX, backY] = pathf::getBackGLoc(coLoc.x, coLoc.y, coLoc.direction, 3);
        co_return co_await requestSpaceMove(backX, backY, false);
    }
}

std::string Player::sendDelivery(std::vector<SDItem> itemList)
{
    auto delivery = dbCreateDelivery(dbid(), std::move(itemList), to_cstr(u8"你收到了一份系统投递："));
    postNetMessage(SM_CHATMESSAGELIST, cerealf::serialize(SDChatMessageList{std::move(delivery.message)}));
    return delivery.record;
}

std::optional<int> Player::claimDelivery(const std::string &record)
{
    // the dbClaim and dbUpdateInventoryItem should be in same database transaction
    // here keep it as it is for simplicity

    if(const auto claimedItemList = dbClaimDelivery(record); claimedItemList.has_value()){
        for(const auto &item: claimedItemList.value()){
            if(item.isGold()){
                addGold(item.count);
            }
            else{
                addInventoryItem(item, false);
            }
        }
        return std::nullopt;
    }
    else{
        return claimedItemList.error();
    }
}
