#include <catch2/catch_test_macros.hpp>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <sol/sol.hpp>
#include "dbpod.hpp"
#include "luaf.hpp"
#include "sysconst.hpp"
#include "questdb.hpp"

class ServerArgParser;
class PeerConfig;
class Log;
class ActorPool;
class MapBinDB;
class ScriptWindow;
class ProfilerWindow;
class MainWindow;
class Server;
class ServerPasswordWindow;
class ServerConfigureWindow;
class PodMonitorWindow;
class ActorMonitorWindow;

ServerArgParser       *g_serverArgParser = nullptr;
PeerConfig            *g_peerConfig = nullptr;
Log                   *g_mir2xLog = nullptr;
ActorPool             *g_actorPool = nullptr;
DBPod                 *g_dbPod = nullptr;
MapBinDB              *g_mapBinDB = nullptr;
ScriptWindow          *g_scriptWindow = nullptr;
ProfilerWindow        *g_profilerWindow = nullptr;
MainWindow            *g_mainWindow = nullptr;
Server                *g_server = nullptr;
ServerPasswordWindow  *g_serverPasswordWindow = nullptr;
ServerConfigureWindow *g_serverConfigureWindow = nullptr;
PodMonitorWindow      *g_podMonitorWindow = nullptr;
ActorMonitorWindow    *g_actorMonitorWindow = nullptr;

namespace
{
    using FieldList = std::vector<std::pair<std::string, std::optional<luaf::luaVar>>>;
    constexpr const char *questTable = "tbl_questdb_test";

    void require(bool condition, const char *message)
    {
        INFO(message);
        REQUIRE(condition);
    }

    template<typename Func> bool raises(Func &&func)
    {
        try{
            func();
        }
        catch(const std::exception &){
            return true;
        }
        return false;
    }

    luaf::luaVar statesOf(const char *state)
    {
        return luaf::buildLuaVar(std::unordered_map<std::string, std::vector<std::string>>
        {
            {SYS_QSTFSM, {state}},
        });
    }

    luaf::luaVar contextOf(const char *key, int version)
    {
        return luaf::buildLuaVar(std::unordered_map<std::string, std::unordered_map<std::string, lua_Integer>>
        {
            {key, {{"version", version}}},
        });
    }

    bool fieldIs(uint32_t dbid, const char *field, const std::optional<luaf::luaVar> &expected)
    {
        return dbLoadQuestField(questTable, dbid, field) == expected;
    }

    void setupDatabase()
    {
        g_dbPod->launch(":memory:");
        g_dbPod->exec("pragma foreign_keys = on");
        g_dbPod->exec("create table tbl_char(fld_dbid integer primary key)");
        g_dbPod->exec("insert into tbl_char(fld_dbid) values(1), (2), (3)");
    }

    void testCreateTable()
    {
        dbCreateQuestTable(questTable);
        dbCreateQuestTable(questTable);

        std::vector<std::string> columns;
        auto query = g_dbPod->createQuery("pragma table_info(%s)", questTable);
        while(query.executeStep()){
            columns.push_back(query.getColumn("name").getString());
        }
        require(columns == std::vector<std::string>{"fld_dbid", "fld_timestamp", "fld_states", "fld_flags", "fld_team", "fld_vars", "fld_desp", "fld_context"}, "quest table doesn't have exactly the columns of the quest context");
    }

    void testWriteAndLoad()
    {
        dbUpdateQuestFields(questTable, 1, FieldList{{"fld_states", statesOf("a")}, {"fld_context", contextOf("npc/m/n", 1)}}, false);
        require(fieldIs(1, "fld_states", statesOf("a")) && fieldIs(1, "fld_context", contextOf("npc/m/n", 1)), "a new row doesn't get both fields");
        require(fieldIs(1, "fld_vars", std::nullopt), "a field never written isn't null");

        dbUpdateQuestFields(questTable, 1, FieldList{{"fld_vars", luaf::buildLuaVar(std::string("v"))}}, false);
        require(fieldIs(1, "fld_vars", luaf::buildLuaVar(std::string("v"))), "an update doesn't write its field");
        require(fieldIs(1, "fld_states", statesOf("a")) && fieldIs(1, "fld_context", contextOf("npc/m/n", 1)), "an update of one field changes the others");

        dbUpdateQuestFields(questTable, 1, FieldList{{"fld_states", statesOf("b")}, {"fld_context", std::nullopt}}, false);
        require(fieldIs(1, "fld_states", statesOf("b")) && fieldIs(1, "fld_context", std::nullopt), "a field without value isn't set to null");
        require(fieldIs(1, "fld_vars", luaf::buildLuaVar(std::string("v"))), "setting a field to null changes the others");
        require(fieldIs(2, "fld_states", std::nullopt), "a write changes another row");
    }

    void testReplace()
    {
        dbUpdateQuestFields(questTable, 2, FieldList{{"fld_states", statesOf("a")}, {"fld_vars", luaf::buildLuaVar(std::string("v"))}, {"fld_context", contextOf("grid/door", 3)}}, false);
        dbUpdateQuestFields(questTable, 2, FieldList{{"fld_states", statesOf(SYS_DONE)}}, true);

        require(fieldIs(2, "fld_states", statesOf(SYS_DONE)), "a replace doesn't write its field");
        require(fieldIs(2, "fld_vars", std::nullopt) && fieldIs(2, "fld_context", std::nullopt), "a replace keeps the fields it doesn't give");
        require(fieldIs(1, "fld_states", statesOf("b")), "a replace changes another row");
    }

    void testAtomic()
    {
        // the second field of the statement fails, the first one isn't written either
        g_dbPod->exec(
            "create temp trigger fail_update before update on %s when new.fld_context is not null"
            " begin select raise(abort, 'injected context failure'); end", questTable);

        dbUpdateQuestFields(questTable, 3, FieldList{{"fld_states", statesOf("a")}}, false);
        require(raises([]{ dbUpdateQuestFields(questTable, 3, FieldList{{"fld_states", statesOf("b")}, {"fld_context", contextOf("npc/m/n", 2)}}, false); }), "a failed write isn't reported");
        g_dbPod->exec("drop trigger fail_update");
        require(fieldIs(3, "fld_states", statesOf("a")) && fieldIs(3, "fld_context", std::nullopt), "a failed write keeps one of its fields");

        g_dbPod->exec(
            "create temp trigger fail_replace before insert on %s when new.fld_states is not null"
            " begin select raise(abort, 'injected done failure'); end", questTable);

        require(raises([]{ dbUpdateQuestFields(questTable, 1, FieldList{{"fld_states", statesOf(SYS_DONE)}}, true); }), "a failed replace isn't reported");
        g_dbPod->exec("drop trigger fail_replace");
        require(fieldIs(1, "fld_states", statesOf("b")) && fieldIs(1, "fld_vars", luaf::buildLuaVar(std::string("v"))), "a failed replace changes the row");
    }

    void testFieldNames()
    {
        // field names are put in the sql text
        require(raises([]{ dbUpdateQuestFields(questTable, 1, FieldList{{"states", statesOf("a")}}, false); }), "a field name without fld_ is accepted");
        require(raises([]{ dbUpdateQuestFields(questTable, 1, FieldList{{"fld_dbid", luaf::buildLuaVar(lua_Integer(9))}}, false); }), "fld_dbid is accepted as a field");
        require(raises([]{ dbUpdateQuestFields(questTable, 1, FieldList{{"fld_timestamp", luaf::buildLuaVar(lua_Integer(9))}}, false); }), "fld_timestamp is accepted as a field");
        require(raises([]{ dbUpdateQuestFields(questTable, 1, FieldList{{"fld_vars=null, fld_states", statesOf("a")}}, false); }), "a field name with sql in it is accepted");
        require(raises([]{ dbUpdateQuestFields(questTable, 1, FieldList{{"fld_vars", std::nullopt}, {"fld_vars", std::nullopt}}, false); }), "a field given twice is accepted");
        require(raises([]{ dbUpdateQuestFields(questTable, 1, FieldList{}, false); }), "a write of no field is accepted");
        require(raises([]{ dbLoadQuestField(questTable, 1, "fld_states from tbl_char --"); }), "a field name with sql in it is accepted by a load");
        require(fieldIs(1, "fld_states", statesOf("b")) && fieldIs(1, "fld_vars", luaf::buildLuaVar(std::string("v"))), "a refused write changes the row");
    }

    void testLuaFieldList()
    {
        sol::state lua;
        lua["SYS_LUANIL"] = SYS_LUANIL;

        const sol::table fieldTable = lua.safe_script(R"###(
            return {fld_vars = SYS_LUANIL, fld_flags = {done = 1}, fld_desp = 'text'}
        )###");

        std::map<std::string, std::optional<luaf::luaVar>> fields;
        for(auto &[name, value]: buildQuestFieldList(fieldTable)){
            fields.emplace(name, std::move(value));
        }

        require(fields.size() == 3, "the field list doesn't have every field of the table");
        require(fields.contains("fld_vars") && !fields.at("fld_vars").has_value(), "SYS_LUANIL isn't a field without value");
        require(fields.contains("fld_desp") && (fields.at("fld_desp") == luaf::buildLuaVar(std::string("text"))), "a string field isn't kept");
        require(fields.contains("fld_flags") && fields.at("fld_flags").has_value() && (luaf::luaVarAs<std::map<std::string, lua_Integer>>(fields.at("fld_flags").value()) == std::map<std::string, lua_Integer>{{"done", 1}}), "a table field isn't kept");

        const sol::table badTable = lua.safe_script("return {[1] = 'x'}");
        require(raises([&badTable]{ buildQuestFieldList(badTable); }), "a field list with a key that isn't a string is accepted");
    }


    struct DatabaseTestEnvironment
    {
        DBPod database;

        struct ResetGlobal
        {
            ~ResetGlobal()
            {
                g_dbPod = nullptr;
            }
        } resetGlobal;

        DatabaseTestEnvironment()
        {
            g_dbPod = &database;
            setupDatabase();
        }
    };

    void prepareWrittenQuestRow()
    {
        dbCreateQuestTable(questTable);
        dbUpdateQuestFields(questTable, 1, FieldList{{"fld_states", statesOf("b")}, {"fld_vars", luaf::buildLuaVar(std::string("v"))}}, false);
    }
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Create Table", "[unit][questdb]")
{
    testCreateTable();
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Write And Load", "[unit][questdb]")
{
    dbCreateQuestTable(questTable);
    testWriteAndLoad();
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Replace", "[unit][questdb]")
{
    prepareWrittenQuestRow();
    testReplace();
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Atomic", "[unit][questdb]")
{
    prepareWrittenQuestRow();
    testAtomic();
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Field Names", "[unit][questdb]")
{
    prepareWrittenQuestRow();
    testFieldNames();
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Lua Field List", "[unit][questdb]")
{
    testLuaFieldList();
}
