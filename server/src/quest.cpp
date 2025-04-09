#include "luaf.hpp"
#include "uidf.hpp"
#include "strf.hpp"
#include "quest.hpp"
#include "dbpod.hpp"
#include "filesys.hpp"
#include "monoserver.hpp"

extern DBPod *g_dbPod;
extern MonoServer *g_monoServer;

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

    bindFunction("_RSVD_NAME_setQuestDesp", [this](uint64_t uid, sol::object despTable, std::string fsm, sol::object desp)
    {
        fflassert(str_haschar(fsm));
        fflassert(desp.is<std::string>() || (desp == sol::nil), luaf::luaObjTypeString(desp));

        const auto dbName = getQuest()->getQuestDBName();
        const auto dbid = uidf::getPlayerDBID(uid);
        const auto timestamp = hres_tstamp().to_nsec();

        if(despTable == sol::nil){
            g_dbPod->exec(
                u8R"###( insert into %s(fld_dbid, fld_timestamp, fld_desp) )###"
                u8R"###( values                                            )###"
                u8R"###(     (%llu, %llu, null)                            )###"
                u8R"###(                                                   )###"
                u8R"###( on conflict(fld_dbid) do                          )###"
                u8R"###( update set                                        )###"
                u8R"###(                                                   )###"
                u8R"###(     fld_timestamp=%llu,                           )###"
                u8R"###(     fld_desp=null                                 )###",

                dbName.c_str(),

                to_llu(dbid),
                to_llu(timestamp),
                to_llu(timestamp));
        }
        else if(despTable.is<sol::table>()){
            auto query = g_dbPod->createQuery(
                u8R"###( insert into %s(fld_dbid, fld_timestamp, fld_desp) )###"
                u8R"###( values                                            )###"
                u8R"###(     (%llu, %llu, ?)                               )###"
                u8R"###(                                                   )###"
                u8R"###( on conflict(fld_dbid) do                          )###"
                u8R"###( update set                                        )###"
                u8R"###(                                                   )###"
                u8R"###(     fld_timestamp=%llu,                           )###"
                u8R"###(     fld_desp=excluded.fld_desp                    )###",

                dbName.c_str(),

                to_llu(dbid),
                to_llu(timestamp),
                to_llu(timestamp));

            query.bindBlob(1, cerealf::serialize(luaf::buildLuaVar(despTable)));
            query.exec();
        }
        else{
            throw fflerror("invalid type: %s", to_cstr(luaf::luaObjTypeString(despTable)));
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
        sol::state_view sv(s);
        const auto dbName = getQuest()->getQuestDBName();
        const auto dbid = uidf::getPlayerDBID(uid);

        fflassert(str_haschar(fieldName));
        fflassert(fieldName.starts_with("fld_"));

        auto queryStatement = g_dbPod->createQuery(u8R"###(select %s from %s where fld_dbid=%llu and %s is not null)###", fieldName.c_str(), dbName.c_str(), to_llu(dbid), fieldName.c_str());
        if(!queryStatement.executeStep()){
            return sol::make_object(sv, sol::nil);
        }
        return luaf::buildLuaObj(sol::state_view(s), cerealf::deserialize<luaf::luaVar>(queryStatement.getColumn(0)));
    });

    bindFunction("dbSetQuestField", [this](uint64_t uid, std::string fieldName, sol::object obj)
    {
        const auto dbName = getQuest()->getQuestDBName();
        const auto dbid = uidf::getPlayerDBID(uid);
        const auto timestamp = hres_tstamp().to_nsec();

        fflassert(str_haschar(fieldName));
        fflassert(fieldName.starts_with("fld_"));

        if(obj == sol::nil){
            g_dbPod->exec(
                u8R"###( insert into %s(fld_dbid, fld_timestamp, %s) )###"
                u8R"###( values                                      )###"
                u8R"###(     (%llu, %llu, null)                      )###"
                u8R"###(                                             )###"
                u8R"###( on conflict(fld_dbid) do                    )###"
                u8R"###( update set                                  )###"
                u8R"###(                                             )###"
                u8R"###(     fld_timestamp=%llu,                     )###"
                u8R"###(     %s=null                                 )###",

                dbName.c_str(),
                fieldName.c_str(),

                to_llu(dbid),
                to_llu(timestamp),

                to_llu(timestamp),
                fieldName.c_str());
        }
        else{
            auto query = g_dbPod->createQuery(
                u8R"###( insert into %s(fld_dbid, fld_timestamp, %s) )###"
                u8R"###( values                                      )###"
                u8R"###(     (%llu, %llu, ?)                         )###"
                u8R"###(                                             )###"
                u8R"###( on conflict(fld_dbid) do                    )###"
                u8R"###( update set                                  )###"
                u8R"###(                                             )###"
                u8R"###(     fld_timestamp=%llu,                     )###"
                u8R"###(     %s=excluded.%s                          )###",

                dbName.c_str(),
                fieldName.c_str(),

                to_llu(dbid),
                to_llu(timestamp),

                to_llu(timestamp),
                fieldName.c_str(),
                fieldName.c_str());

            query.bindBlob(1, cerealf::serialize(luaf::buildLuaVar(obj)));
            query.exec();
        }
    });

    bindFunction("_RSVD_NAME_dbSetQuestStateDone", [this](uint64_t uid)
    {
        // finialize quest
        // all quest vars get removed except fld_states

        const auto dbName = getQuest()->getQuestDBName();
        const auto dbid = uidf::getPlayerDBID(uid);
        const auto timestamp = hres_tstamp().to_nsec();

        auto query = g_dbPod->createQuery(
            u8R"###( replace into %s(fld_dbid, fld_timestamp, fld_states) )###"
            u8R"###( values                                               )###"
            u8R"###(     (%llu, %llu, ?)                                  )###",

            dbName.c_str(),

            to_llu(dbid),
            to_llu(timestamp));

        query.bindBlob(1, cerealf::serialize(luaf::buildLuaVar(std::unordered_map<std::string, std::vector<std::string>>
        {
            {SYS_QSTFSM, {SYS_DONE}},
        })));
        query.exec();
    });

    bindFunctionCoop("_RSVD_NAME_modifyQuestTriggerType", [this](LuaCoopResumer onDone, int triggerType, bool enable)
    {
        fflassert(triggerType >= SYS_ON_BEGIN, triggerType);
        fflassert(triggerType <  SYS_ON_END  , triggerType);

        auto closed = std::make_shared<bool>(false);
        onDone.pushOnClose([closed]()
        {
            *closed = true;
        });

        AMModifyQuestTriggerType amMQTT;
        std::memset(&amMQTT, 0, sizeof(amMQTT));

        amMQTT.type = triggerType;
        amMQTT.enable = enable;

        m_actorPod->forward(uidf::getServiceCoreUID(), {AM_MODIFYQUESTTRIGGERTYPE, amMQTT}, [closed, onDone, this](const ActorMsgPack &rmpk)
        {
            if(*closed){
                return;
            }
            else{
                onDone.popOnClose();
            }

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
    });

    bindFunction("_RSVD_NAME_closeQuestState", [this](uint64_t uid, const char *fsm)
    {
        closeQuestState(uid, fsm, m_currRunner);
    });
}

void Quest::LuaThreadRunner::closeQuestState(uint64_t uid, const char *fsm, const void *handle)
{
    fflassert(uidf::isPlayer(uid));
    fflassert(str_haschar(fsm));

    auto &fsmStateRunner = getQuest()->m_uidStateRunner[fsm];
    auto p = fsmStateRunner.find(uid);

    if(p == fsmStateRunner.end()){
        return;
    }

    if(static_cast<const LuaThreadHandle *>(handle)->key == p->second){
        // put current thread to delayed close state
        // however it's not deleted now after this function
        //
        // we also don't have suspend mark for thread running state
        // means before the delayed handler registered here is called, the thread can still be resumed
        //
        // to prevent this, lua code must put an infinite loop after this function call:
        //
        //     while true do
        //         coroutine.yield()
        //     end
        //
        // this prevents any possibility of resuming the thread after this function call

        getQuest()->addDelay(0, [threadKey = p->second, this]()
        {
            close(threadKey);
        });
    }
    else{
        close(p->second);
    }

    fsmStateRunner.erase(p);
}

Quest::Quest(const SDInitQuest &initQuest)
    : ServerObject(uidf::getQuestUID(initQuest.questID))
    , m_scriptName(initQuest.fullScriptName)
{
    if(!g_dbPod->createQuery(u8R"###(select name from sqlite_master where type='table' and name='%s')###", getQuestDBName().c_str()).executeStep()){
        g_dbPod->exec(
            u8R"###( create table %s(                                          )###"
            u8R"###(     fld_dbid         int unsigned not null,               )###"
            u8R"###(     fld_timestamp    int unsigned not null,               )###"
            u8R"###(     fld_states       blob             null,               )###"
            u8R"###(     fld_flags        blob             null,               )###"
            u8R"###(     fld_team         blob             null,               )###"
            u8R"###(     fld_vars         blob             null,               )###"
            u8R"###(     fld_desp         blob             null,               )###"
            u8R"###(     fld_npcbehaviors blob             null,               )###"
            u8R"###(                                                           )###"
            u8R"###(     foreign key (fld_dbid) references tbl_char(fld_dbid), )###"
            u8R"###(     primary key (fld_dbid)                                )###"
            u8R"###( );                                                        )###", getQuestDBName().c_str());
    }
}

void Quest::onActivate()
{
    ServerObject::onActivate();
    m_actorPod->forward(uidf::getServiceCoreUID(), {AM_REGISTERQUEST, cerealf::serialize(SDRegisterQuest
    {
        .name = getQuestName(),
    })});

    m_luaRunner = std::make_unique<Quest::LuaThreadRunner>(this);

    m_luaRunner->pfrCheck(m_luaRunner->execRawString(BEGIN_LUAINC(char)
#include "quest.lua"
    END_LUAINC()));

    m_luaRunner->spawn(m_mainScriptThreadKey, filesys::readFile(m_scriptName.c_str()));
}

void Quest::operateAM(const ActorMsgPack &mpk)
{
    switch(mpk.type()){
        case AM_METRONOME:
            {
                on_AM_METRONOME(mpk);
                break;
            }
        case AM_REMOTECALL:
            {
                on_AM_REMOTECALL(mpk);
                break;
            }
        case AM_RUNQUESTTRIGGER:
            {
                on_AM_RUNQUESTTRIGGER(mpk);
                break;
            }
        default:
            {
                throw fflerror("unsupported message: %s", mpkName(mpk.type()));
            }
    }
}

void Quest::dumpQuestField(uint64_t uid, const std::string &fieldName) const
{
    const auto dbName = getQuestDBName();
    const auto dbid = uidf::getPlayerDBID(uid);

    fflassert(str_haschar(fieldName));
    fflassert(fieldName.starts_with("fld_"));

    auto queryStatement = g_dbPod->createQuery(u8R"###(select %s from %s where fld_dbid=%llu and %s is not null)###", fieldName.c_str(), dbName.c_str(), to_llu(dbid), fieldName.c_str());
    if(queryStatement.executeStep()){
        std::cout << str_printf("table %s, uid %llu, dbid %llu, field %s: %s", dbName.c_str(), to_llu(uid), to_llu(dbid), fieldName.c_str(), str_any(cerealf::deserialize<luaf::luaVar>(queryStatement.getColumn(0))).c_str()) << std::endl;
    }
    else{
        std::cout << str_printf("table %s, uid %llu, dbid %llu, field %s: no result", dbName.c_str(), to_llu(uid), to_llu(dbid), fieldName.c_str()) << std::endl;
    }
}
