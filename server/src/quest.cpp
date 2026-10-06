#include "luaf.hpp"
#include "uidf.hpp"
#include "strf.hpp"
#include "totype.hpp"
#include "quest.hpp"
#include "questdb.hpp"
#include "filesys.hpp"
#include "server.hpp"

extern Server *g_server;

Quest::LuaThreadRunner::LuaThreadRunner(Quest *quest)
    : ServerObject::LuaThreadRunner(quest)
{
    fflassert(dynamic_cast<Quest *>(getSO()));
    fflassert(dynamic_cast<Quest *>(getSO()) == quest);

    bindFunction("getQuestName", [this]() -> std::string
    {
        return getQuest()->getQuestName();
    });

    bindFunction("getMainScriptThreadKey", [this]() -> uint64_t
    {
        return getQuest()->m_mainScriptThreadKey;
    });

    bindFunction("rollKey", [this]() -> uint64_t
    {
        return getQuest()->m_threadKey++;
    });

    // saved with each quest context item, an item saved by another version of the script can run code that changed since
    bindFunction("getQuestScriptHash", [this]() -> std::string
    {
        return getQuest()->m_scriptHash;
    });

    bindFunction("_RSVD_NAME_setQuestDesp", [this](uint64_t uid, sol::object despTable, std::string fsm, sol::object desp)
    {
        fflassert(str_haschar(fsm));
        fflassert(desp.is<std::string>() || (desp == sol::lua_nil), luaf::luaObjTypeString(desp));

        if(despTable == sol::lua_nil){
            dbUpdateQuestFields(getQuest()->getQuestDBName(), uidf::getPlayerDBID(uid), {{"fld_desp", std::nullopt}}, false);
        }
        else if(despTable.is<sol::table>()){
            dbUpdateQuestFields(getQuest()->getQuestDBName(), uidf::getPlayerDBID(uid), {{"fld_desp", luaf::buildLuaVar(despTable)}}, false);
        }
        else{
            throw fflpanic("invalid type: {}", to_cstr(luaf::luaObjTypeString(despTable)));
        }

        SDQuestDespUpdate sdQDU
        {
            .name = getQuest()->getQuestName(),
            .fsm  = fsm,
            .desp = desp.is<std::string>() ? std::make_optional<std::string>(desp.as<std::string>()) : std::nullopt,
        };

        getQuest()->forwardNetPackage(uid, SM_QUESTDESPUPDATE, cerealf::serialize(sdQDU));
    });

    bindFunction("dbGetQuestField", [this](uint64_t uid, std::string fieldName, sol::this_state s) -> sol::object
    {
        if(const auto value = dbLoadQuestField(getQuest()->getQuestDBName(), uidf::getPlayerDBID(uid), fieldName)){
            return luaf::buildLuaObj(sol::state_view(s), value.value());
        }
        return sol::make_object(sol::state_view(s), sol::lua_nil);
    });

    bindFunction("dbSetQuestField", [this](uint64_t uid, std::string fieldName, sol::object obj)
    {
        std::optional<luaf::luaVar> value;
        if(obj != sol::lua_nil){
            value = luaf::buildLuaVar(obj);
        }
        dbUpdateQuestFields(getQuest()->getQuestDBName(), uidf::getPlayerDBID(uid), {{fieldName, std::move(value)}}, false);
    });

    // writes several fields of the row of uid in one statement, a crash keeps all of them or none
    // fieldTable is {fld_xxx = value, ...}, SYS_LUANIL sets a field to null
    // replace: the row is written whole, the fields not given are set to null too, i.e. quest done keeps fld_states only
    bindFunction("_RSVD_NAME_dbSetQuestFields", [this](uint64_t uid, sol::table fieldTable, bool replace)
    {
        dbUpdateQuestFields(getQuest()->getQuestDBName(), uidf::getPlayerDBID(uid), buildQuestFieldList(fieldTable), replace);
    });

    bindCoop("_RSVD_NAME_modifyQuestTriggerType", [thisptr = this](this auto, LuaCoopResumer onDone, int triggerType, bool enable) -> corof::awaitable<>
    {
        fflassert(triggerType >= SYS_ON_BEGIN, triggerType);
        fflassert(triggerType <  SYS_ON_END  , triggerType);

        bool closed = false;
        onDone.pushOnClose([&closed](){ closed = true; });

        AMModifyQuestTriggerType amMQTT;
        std::memset(&amMQTT, 0, sizeof(amMQTT));
        amMQTT.type = triggerType;
        amMQTT.enable = enable;

        const auto rmpk = co_await thisptr->m_actorPod->send(uidf::getServiceCoreUID(), {AM_MODIFYQUESTTRIGGERTYPE, amMQTT});

        if(closed){
            co_return;
        }

        onDone.popOnClose();

        // expected an reply
        // this makes sure when modifyQuestTriggerType() returns, the trigger has already been enabled/disabled

        switch(rmpk.type()){
            case AM_OK:
                {
                    onDone(true);
                    break;
                }
            default:
                {
                    onDone();
                    break;
                }
        }
    });
}

Quest::Quest(const SDInitQuest &initQuest)
    : ServerObject(uidf::getQuestUID(initQuest.questID))
    , m_scriptName(initQuest.fullScriptName)
{
    dbCreateQuestTable(getQuestDBName());
}

corof::awaitable<> Quest::onActivate()
{
    co_await ServerObject::onActivate();
    m_actorPod->post(uidf::getServiceCoreUID(), {AM_REGISTERQUEST, cerealf::serialize(SDRegisterQuest
    {
        .name = getQuestName(),
    })});

    m_luaRunner = std::make_unique<Quest::LuaThreadRunner>(this);

    constexpr static unsigned char luaScript []
    {
        #embed "quest.lua" suffix(,)
        '\0'
    };
    m_luaRunner->pfrCheck(m_luaRunner->execRawString(to_rawcstr(luaScript)));

    // FNV-1a, std::hash isn't the same across builds
    const auto script = filesys::readFile(m_scriptName.c_str());
    uint64_t scriptHash = 14695981039346656037ULL;

    for(const auto ch: script){
        scriptHash ^= static_cast<unsigned char>(ch);
        scriptHash *= 1099511628211ULL;
    }

    m_scriptHash = str_printf("%016llx", to_llu(scriptHash));
    m_luaRunner->spawn(m_mainScriptThreadKey, script);
}

corof::awaitable<> Quest::onActorMsg(const ActorMsgPack &mpk)
{
    switch(mpk.type()){
        case AM_REMOTECALL:
            {
                return on_AM_REMOTECALL(mpk);
            }
        default:
            {
                throw fflpanic("unsupported message: {}", mpkName(mpk.type()));
            }
    }
}

void Quest::dumpQuestField(uint64_t uid, const std::string &fieldName) const
{
    const auto dbName = getQuestDBName();
    const auto dbid = uidf::getPlayerDBID(uid);

    if(const auto value = dbLoadQuestField(dbName, dbid, fieldName)){
        std::cout << str_printf("table %s, uid %llu, dbid %llu, field %s: %s", dbName.c_str(), to_llu(uid), to_llu(dbid), fieldName.c_str(), str_any(value.value()).c_str()) << std::endl;
    }
    else{
        std::cout << str_printf("table %s, uid %llu, dbid %llu, field %s: no result", dbName.c_str(), to_llu(uid), to_llu(dbid), fieldName.c_str()) << std::endl;
    }
}
