#include <cinttypes>
#include "dbpod.hpp"
#include "player.hpp"
#include "luaf.hpp"
#include "uidf.hpp"
#include "uidsf.hpp"
#include "jobf.hpp"
#include "pathf.hpp"
#include "mathf.hpp"
#include "dbcomid.hpp"
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

    bindFunction("getGold", [this]() -> uint64_t
    {
        return getPlayer()->gold();
    });

    bindFunction("getGender", [this]() -> bool
    {
        return getPlayer()->gender();
    });

    bindFunction("getName", [this]() -> std::string
    {
        return getPlayer()->name();
    });

    bindFunction("getWLItem", [this](int wlType, sol::this_state s) -> sol::object
    {
        if(const auto &item = getPlayer()->m_sdItemStorage.wear.getWLItem(wlType)){
            return luaf::buildLuaObj(sol::state_view(s), item.asLuaVar());
        }
        else{
            return sol::make_object(sol::state_view(s), sol::nil);
        }
    });

    bindFunction("getTeamLeader", [this](sol::this_state s) -> sol::object
    {
        sol::state_view sv(s);
        if(getPlayer()->m_teamLeader){
            return sol::object(sv, sol::in_place_type<lua_Integer>, getPlayer()->m_teamLeader);
        }
        else{
            return sol::make_object(sv, sol::nil);
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

    bindFunction("addItem", [this](int itemID, int itemCount)
    {
        const auto &ir = DBCOM_ITEMRECORD(itemID);
        fflassert(ir);
        fflassert(itemCount > 0);

        if(ir.isGold()){
            getPlayer()->setGold(getPlayer()->getGold() + itemCount);
        }
        else{
            int added = 0;
            while(added < itemCount){
                const auto &addedItem = getPlayer()->addInventoryItem(SDItem
                {
                    .itemID = to_u32(itemID),
                    .seqID  = 1,
                    .count = std::min<size_t>(ir.packable() ? SYS_INVGRIDMAXHOLD : 1, itemCount - added),
                }, false);
                added += addedItem.count;
            }
        }
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

    pfrCheck(execRawString(BEGIN_LUAINC(char)
#include "player.lua"
    END_LUAINC()));
}

Player::Player(const SDInitPlayer &initParam)
    : BattleObject(uidf::getPlayerUID(initParam.dbid), initParam.mapUID, initParam.x, initParam.y, DIR_DOWN)
    , m_exp(initParam.exp)
    , m_gender(initParam.gender)
    , m_job(initParam.job)
    , m_name(initParam.name)
    , m_nameColor(initParam.nameColor)
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
    dbLoadInventory();
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
        _support_cm(CM_REQUESTSPACEMOVE          );
        _support_cm(CM_SETMAGICKEY               );
        _support_cm(CM_SETRUNTIMECONFIG          );
        _support_cm(CM_CREATECHATGROUP           );
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

bool Player::struckDamage(uint64_t, const DamageNode &node)
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

    if(damage > 0){
        updateHealth(-damage);
        if(m_sdHealth.dead()){
            onDie();
        }
    }
    return true;
}

bool Player::ActionValid(const ActionNode &)
{
    return true;
}

void Player::dispatchOffline()
{
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
                        .itemID = DBCOM_ITEMID(u8"黑铁"),
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
                                                    scoped_alloc::svobuf_wrapper<std::tuple<int, int>, 3> aimGridList;
                                                    for(int d: {-1, 0, 1}){
                                                        aimGridList.c.push_back(pathf::getFrontGLoc(X(), Y(), pathf::getNextDir(Direction(), d)));
                                                    }

                                                    for(const auto &[uid, coLoc]: m_inViewCOList){
                                                        if(std::find(aimGridList.c.begin(), aimGridList.c.end(), std::make_tuple(coLoc.x, coLoc.y)) != aimGridList.c.end()){
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
                                            if(buffID){
                                                sendBuff(uid, 0, buffID);
                                            }
                                            dispatchAttackDamage(uid, nDCType, 0);
                                        }

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
                                                if(br.favor <= 0){
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

                    addDelay(delay, [cmA, this](bool)
                    {
                        dispatchAttackDamage(cmA.action.aimUID, cmA.action.extParam.spell.magicID, 0);
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
                    addDelay(300, [smFM, this](bool)
                    {
                        dispatchAttackDamage(smFM.AimUID, DBCOM_MAGICID(u8"雷电术"), 0);
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
                                co_return isOffender(targetUID) ? FT_ENEMY : FT_NEUTRAL;
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
        .nameColor = m_nameColor,
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
        SDChatPeerID(CP_SPECIAL, SYS_CHATDBID_SYSTEM).asU64(),
        SDChatPeerID(CP_PLAYER , dbid()             ).asU64(),
    };

    std::for_each(m_sdFriendList.begin(), m_sdFriendList.end(), [&friendIDList](const auto &peer)
    {
        friendIDList.push_back(peer.cpid().asU64());
    });

    if(!friendIDList.empty()){
        postNetMessage(SM_CHATMESSAGELIST, cerealf::serialize(dbRetrieveLatestChatMessage(friendIDList, 1, true, true)));
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
    const auto &addedItem = m_sdItemStorage.inventory.add(std::move(item), keepSeqID);
    dbUpdateInventoryItem(addedItem);

    m_luaRunner->spawn(m_threadKey++, str_printf("_RSVD_NAME_trigger(SYS_ON_GAINITEM, %llu)", to_llu(item.itemID)));

    postNetMessage(SM_UPDATEITEM, cerealf::serialize(SDUpdateItem
    {
        .item = addedItem,
    }));
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
        const auto [removedCount, removedSeqID, itemPtr] = m_sdItemStorage.inventory.remove(itemID, seqID, count - doneCount);
        if(!removedCount){
            break;
        }

        if(itemPtr){
            dbUpdateInventoryItem(*itemPtr);
        }
        else{
            dbRemoveInventoryItem(itemID, removedSeqID);
        }

        doneCount += removedCount;
        reportRemoveItem(itemID, removedSeqID, removedCount);
    }
    return doneCount;
}

const SDItem &Player::findInventoryItem(uint32_t itemID, uint32_t seqID) const
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
    g_dbPod->exec("update tbl_char set fld_gold = %llu where fld_dbid = %llu", to_llu(m_sdItemStorage.gold), to_llu(dbid()));
    reportGold();
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
        throw fflerror("bad wltype: %d", wltype);
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

    const auto magicID = DBCOM_MAGICID(ir.name);
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
            if(jobstr){
                if(to_u8sv(jobstr) == mr.req.job){
                    hasJob = true;
                }
            }
            else{
                break;
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
