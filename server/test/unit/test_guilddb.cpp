#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include "dbpod.hpp"
#include "totype.hpp"
#include "sysconst.hpp"
#include "protocoldef.hpp"
#include "guilddb.hpp"

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
            g_dbPod->launch(":memory:");
            g_dbPod->exec("pragma foreign_keys = on");

            // the real schema, with stubs of the functions Server::createDBConnection() registers for it
            g_dbPod->createFunction("jobValid"     , 1, true, +[](sqlite3_context *context, int, sqlite3_value **){ sqlite3_result_int(context, 1); });
            g_dbPod->createFunction("SYS_PLY_VIEWR", 0, true, +[](sqlite3_context *context, int, sqlite3_value **){ sqlite3_result_int(context, 1); });

            constexpr static unsigned char sqlCmd[]
            {
                #embed "../../src/server.sql" suffix(,)
                '\0'
            };
            g_dbPod->exec("%s", to_rawcstr(sqlCmd));

            for(int dbid = 1; dbid <= 5; ++dbid){
                g_dbPod->exec("insert into tbl_account(fld_account, fld_password) values ('id_%d', 'pwd')", dbid);
                g_dbPod->exec("insert into tbl_char(fld_dbid, fld_account, fld_name, fld_gender, fld_job, fld_map, fld_mapx, fld_mapy) values (%d, 'id_%d', 'char_%d', 0, 1, 0, 0, 0)", dbid, dbid, dbid);
            }
        }
    };

    int countRows(const char *table)
    {
        auto query = g_dbPod->createQuery("select count(*) from %s", table);
        REQUIRE(query.executeStep());
        return query.getColumn(0).getInt();
    }

    uint32_t createGuild(uint32_t dbid, const char8_t *guildName)
    {
        const auto result = dbCreateGuild(dbid, to_cstr(guildName));
        REQUIRE(result.has_value());
        return result.value();
    }

    int errorOf(const auto &result)
    {
        REQUIRE(!result.has_value());
        return result.error();
    }
}

TEST_CASE("Guild Name", "[unit][guilddb]")
{
    CHECK(guildNameValid(to_cstr(u8"沙巴克")));
    CHECK(guildNameValid("abc"));

    CHECK(!guildNameValid(""));
    CHECK(!guildNameValid("a b"));
    CHECK(!guildNameValid("a\tb"));
    CHECK(!guildNameValid("a<b"));
    CHECK(!guildNameValid("a&b"));
    CHECK(!guildNameValid("a\"b"));
    CHECK(!guildNameValid(std::string(SYS_GUILDNAMESIZE, 'a')));
    CHECK( guildNameValid(std::string(SYS_GUILDNAMESIZE - 1, 'a')));
    CHECK(!guildNameValid("\xE6\xB2")); // cut utf-8
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Create", "[unit][guilddb]")
{
    const auto guildID = createGuild(1, u8"沙巴克");

    const auto guild = dbLoadGuildMembership(1);
    REQUIRE(guild.has_value());
    CHECK(guild->guildID == guildID);
    CHECK(guild->guildName == to_cstr(u8"沙巴克"));
    CHECK(guild->rank == GUILDRANK_LEADER);
    CHECK(guild->title == to_cstr(u8"掌门人"));
    CHECK(dbLoadGuildID(1) == guildID);

    CHECK(errorOf(dbCreateGuild(1, "other")) == GUILDERR_INGUILD);
    CHECK(errorOf(dbCreateGuild(2, to_cstr(u8"沙巴克"))) == GUILDERR_NAMETAKEN);
    CHECK(errorOf(dbCreateGuild(2, "a b")) == GUILDERR_BADNAME);

    CHECK(!dbLoadGuildMembership(2).has_value());
    CHECK(dbLoadGuildID(2) == 0);
    CHECK(countRows("tbl_guild") == 1);
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Add Member", "[unit][guilddb]")
{
    const auto guildID = createGuild(1, u8"沙巴克");

    CHECK(dbAddGuildMember(1, 1) == GUILDERR_BADTARGET);
    CHECK(dbAddGuildMember(1, 0) == GUILDERR_BADTARGET);
    CHECK(dbAddGuildMember(3, 2) == GUILDERR_NOGUILD);

    CHECK(dbAddGuildMember(1, 2) == GUILDERR_NONE);
    CHECK(dbLoadGuildID(2) == guildID);
    CHECK(dbLoadGuildMembership(2)->rank == GUILDRANK_MEMBER);
    CHECK(dbLoadGuildMembership(2)->title == to_cstr(u8"成员"));

    CHECK(dbAddGuildMember(2, 3) == GUILDERR_NOTLEADER);
    CHECK(dbAddGuildMember(1, 2) == GUILDERR_TARGETINGUILD);

    createGuild(3, u8"另一个");
    CHECK(dbAddGuildMember(1, 3) == GUILDERR_TARGETINGUILD);
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Leave And Kick", "[unit][guilddb]")
{
    createGuild(1, u8"沙巴克");
    REQUIRE(dbAddGuildMember(1, 2) == GUILDERR_NONE);
    REQUIRE(dbAddGuildMember(1, 3) == GUILDERR_NONE);

    CHECK(dbLeaveGuild(1) == GUILDERR_LEADERLEAVE);
    CHECK(dbLeaveGuild(4) == GUILDERR_NOGUILD);
    CHECK(dbLeaveGuild(2) == GUILDERR_NONE);
    CHECK(dbLoadGuildID(2) == 0);

    CHECK(dbKickGuildMember(3, 1) == GUILDERR_NOTLEADER);
    CHECK(dbKickGuildMember(1, 1) == GUILDERR_BADTARGET);
    CHECK(dbKickGuildMember(1, 2) == GUILDERR_NOTMEMBER);

    createGuild(4, u8"另一个");
    CHECK(dbKickGuildMember(1, 4) == GUILDERR_NOTMEMBER);
    CHECK(dbLoadGuildID(4) != 0);

    CHECK(dbKickGuildMember(1, 3) == GUILDERR_NONE);
    CHECK(dbLoadGuildID(3) == 0);
    CHECK(dbLoadGuildID(1) != 0);
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Transfer Leader", "[unit][guilddb]")
{
    const auto guildID = createGuild(1, u8"沙巴克");
    REQUIRE(dbAddGuildMember(1, 2) == GUILDERR_NONE);

    CHECK(dbTransferGuildLeader(1, 1) == GUILDERR_BADTARGET);
    CHECK(dbTransferGuildLeader(1, 3) == GUILDERR_NOTMEMBER);
    CHECK(dbTransferGuildLeader(2, 1) == GUILDERR_NOTLEADER);

    CHECK(dbTransferGuildLeader(1, 2) == GUILDERR_NONE);
    CHECK(dbLoadGuildMembership(1)->rank == GUILDRANK_MEMBER);
    CHECK(dbLoadGuildMembership(1)->title == to_cstr(u8"成员"));
    CHECK(dbLoadGuildMembership(2)->rank == GUILDRANK_LEADER);
    CHECK(dbLoadGuildMembership(2)->title == to_cstr(u8"掌门人"));
    CHECK(dbLoadGuildID(1) == guildID);

    // the old leader is a plain member now
    CHECK(dbAddGuildMember(1, 3) == GUILDERR_NOTLEADER);
    CHECK(dbLeaveGuild(1) == GUILDERR_NONE);

    // a second leader is refused by the database itself
    REQUIRE(dbAddGuildMember(2, 3) == GUILDERR_NONE);
    CHECK_THROWS(g_dbPod->exec("update tbl_guildmember set fld_rank = %d where fld_dbid = 3", to_d(GUILDRANK_LEADER)));
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Disband", "[unit][guilddb]")
{
    createGuild(1, u8"沙巴克");
    REQUIRE(dbAddGuildMember(1, 2) == GUILDERR_NONE);
    REQUIRE(dbAddGuildMember(1, 3) == GUILDERR_NONE);
    createGuild(4, u8"另一个");

    CHECK(errorOf(dbDisbandGuild(2)) == GUILDERR_NOTLEADER);
    CHECK(errorOf(dbDisbandGuild(5)) == GUILDERR_NOGUILD);

    auto memberList = dbDisbandGuild(1);
    REQUIRE(memberList.has_value());
    std::ranges::sort(memberList.value());
    CHECK(memberList.value() == std::vector<uint32_t>{2, 3});

    CHECK(dbLoadGuildID(1) == 0);
    CHECK(dbLoadGuildID(2) == 0);
    CHECK(dbLoadGuildID(3) == 0);
    CHECK(dbLoadGuildID(4) != 0);
    CHECK(countRows("tbl_guild") == 1);

    // the name is free again
    createGuild(2, u8"沙巴克");
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Delete Leader Char", "[unit][guilddb]")
{
    createGuild(1, u8"沙巴克");
    REQUIRE(dbAddGuildMember(1, 2) == GUILDERR_NONE);
    createGuild(3, u8"另一个");

    g_dbPod->exec("delete from tbl_char where fld_dbid = 1");
    CHECK(dbLoadGuildID(2) == 0);
    CHECK(dbLoadGuildID(3) != 0);
    CHECK(countRows("tbl_guild") == 1);

    // deleting a member's char keeps the guild
    REQUIRE(dbAddGuildMember(3, 4) == GUILDERR_NONE);
    g_dbPod->exec("delete from tbl_char where fld_dbid = 4");
    CHECK(dbLoadGuildID(3) != 0);
    CHECK(countRows("tbl_guildmember") == 1);
}
