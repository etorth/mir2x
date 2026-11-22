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

    int addMember(uint32_t leaderDBID, uint32_t dbid)
    {
        if(const auto error = dbInviteGuildMember(leaderDBID, dbid); error != GUILDERR_NONE){
            return error;
        }
        return dbAcceptGuildInvite(dbid, dbLoadGuildID(leaderDBID));
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

    CHECK(addMember(1, 1) == GUILDERR_BADTARGET);
    CHECK(addMember(1, 0) == GUILDERR_BADTARGET);
    CHECK(addMember(3, 2) == GUILDERR_NOGUILD);

    CHECK(addMember(1, 2) == GUILDERR_NONE);
    CHECK(dbLoadGuildID(2) == guildID);
    CHECK(dbLoadGuildMembership(2)->rank == GUILDRANK_MEMBER);
    CHECK(dbLoadGuildMembership(2)->title == to_cstr(u8"成员"));

    CHECK(addMember(2, 3) == GUILDERR_NOTLEADER);
    CHECK(addMember(1, 2) == GUILDERR_TARGETINGUILD);

    createGuild(3, u8"另一个");
    CHECK(addMember(1, 3) == GUILDERR_TARGETINGUILD);
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Leave And Kick", "[unit][guilddb]")
{
    createGuild(1, u8"沙巴克");
    REQUIRE(addMember(1, 2) == GUILDERR_NONE);
    REQUIRE(addMember(1, 3) == GUILDERR_NONE);

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
    REQUIRE(addMember(1, 2) == GUILDERR_NONE);

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
    CHECK(addMember(1, 3) == GUILDERR_NOTLEADER);
    CHECK(dbLeaveGuild(1) == GUILDERR_NONE);

    // a second leader is refused by the database itself
    REQUIRE(addMember(2, 3) == GUILDERR_NONE);
    CHECK_THROWS(g_dbPod->exec("update tbl_guildmember set fld_rank = %d where fld_dbid = 3", to_d(GUILDRANK_LEADER)));
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Disband", "[unit][guilddb]")
{
    createGuild(1, u8"沙巴克");
    REQUIRE(addMember(1, 2) == GUILDERR_NONE);
    REQUIRE(addMember(1, 3) == GUILDERR_NONE);
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
    REQUIRE(addMember(1, 2) == GUILDERR_NONE);
    createGuild(3, u8"另一个");

    g_dbPod->exec("delete from tbl_char where fld_dbid = 1");
    CHECK(dbLoadGuildID(2) == 0);
    CHECK(dbLoadGuildID(3) != 0);
    CHECK(countRows("tbl_guild") == 1);

    // deleting a member's char keeps the guild
    REQUIRE(addMember(3, 4) == GUILDERR_NONE);
    g_dbPod->exec("delete from tbl_char where fld_dbid = 4");
    CHECK(dbLoadGuildID(3) != 0);
    CHECK(countRows("tbl_guildmember") == 1);
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Invite", "[unit][guilddb]")
{
    const auto guildID = createGuild(1, u8"沙巴克");
    const auto otherID = createGuild(4, u8"另一个");

    CHECK(dbAcceptGuildInvite(2, guildID) == GUILDERR_NOINVITE);

    // reject tells who invited, and only once
    REQUIRE(dbInviteGuildMember(1, 2) == GUILDERR_NONE);
    CHECK(dbRejectGuildInvite(2, guildID) == std::optional<uint32_t>(1));
    CHECK(dbRejectGuildInvite(2, guildID) == std::nullopt);
    CHECK(dbAcceptGuildInvite(2, guildID) == GUILDERR_NOINVITE);

    // joining one guild drops the invites from the others
    REQUIRE(dbInviteGuildMember(1, 2) == GUILDERR_NONE);
    REQUIRE(dbInviteGuildMember(4, 2) == GUILDERR_NONE);
    CHECK(dbAcceptGuildInvite(2, otherID) == GUILDERR_NONE);
    CHECK(dbAcceptGuildInvite(2, guildID) == GUILDERR_NOINVITE);
    CHECK(dbLoadGuildID(2) == otherID);
    CHECK(countRows("tbl_guildinvite") == 0);

    // an invite dies with its guild
    REQUIRE(dbInviteGuildMember(1, 3) == GUILDERR_NONE);
    REQUIRE(dbDisbandGuild(1).has_value());
    CHECK(dbAcceptGuildInvite(3, guildID) == GUILDERR_NOINVITE);
    CHECK(countRows("tbl_guildinvite") == 0);

    // creating a guild drops the invites too
    REQUIRE(dbInviteGuildMember(4, 3) == GUILDERR_NONE);
    createGuild(3, u8"第三个");
    CHECK(countRows("tbl_guildinvite") == 0);
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Notice And Title", "[unit][guilddb]")
{
    const auto guildID = createGuild(1, u8"沙巴克");
    REQUIRE(addMember(1, 2) == GUILDERR_NONE);
    REQUIRE(addMember(1, 3) == GUILDERR_NONE);

    CHECK(dbSetGuildNotice(1, to_cstr(u8"今晚攻城\n全员到齐")) == GUILDERR_NONE);
    CHECK(dbSetGuildNotice(2, "x") == GUILDERR_NOTLEADER);
    CHECK(dbSetGuildNotice(1, "a\tb") == GUILDERR_BADNOTICE);
    CHECK(dbSetGuildNotice(1, std::string(SYS_GUILDNOTICESIZE, 'a')) == GUILDERR_BADNOTICE);

    CHECK(dbSetGuildMemberTitle(1, 2, to_cstr(u8"护法")) == GUILDERR_NONE);
    CHECK(dbSetGuildMemberTitle(1, 1, to_cstr(u8"帮主")) == GUILDERR_NONE);
    CHECK(dbSetGuildMemberTitle(2, 3, "x") == GUILDERR_NOTLEADER);
    CHECK(dbSetGuildMemberTitle(1, 4, "x") == GUILDERR_NOTMEMBER);
    CHECK(dbSetGuildMemberTitle(1, 3, "a b") == GUILDERR_BADTITLE);
    CHECK(dbLoadGuildMembership(2)->title == to_cstr(u8"护法"));

    const auto info = dbLoadGuildInfo(guildID);
    CHECK(info.guildID == guildID);
    CHECK(info.name == to_cstr(u8"沙巴克"));
    CHECK(info.notice == to_cstr(u8"今晚攻城\n全员到齐"));

    REQUIRE(info.memberList.size() == 3);
    CHECK(info.memberList[0].dbid == 1);
    CHECK(info.memberList[0].name == "char_1");
    CHECK(info.memberList[0].rank == GUILDRANK_LEADER);
    CHECK(info.memberList[0].title == to_cstr(u8"帮主"));
    CHECK(info.memberList[1].rank == GUILDRANK_MEMBER);

    CHECK(dbLoadGuildInfo(guildID + 100).guildID == 0);
    CHECK(dbFindCharDBID("char_3") == 3);
    CHECK(dbFindCharDBID("nobody") == 0);
}

TEST_CASE_METHOD(DatabaseTestEnvironment, "Guild War", "[unit][guilddb]")
{
    const auto otherID = createGuild(3, u8"另一个");
    const auto guildID = createGuild(1, u8"沙巴克");
    REQUIRE(addMember(1, 2) == GUILDERR_NONE);
    REQUIRE(guildID > otherID); // the pair gets stored in reversed order

    constexpr uint64_t now = 1000;
    CHECK(errorOf(dbDeclareGuildWar(2, to_cstr(u8"另一个"), now)) == GUILDERR_NOTLEADER);
    CHECK(errorOf(dbDeclareGuildWar(4, to_cstr(u8"另一个"), now)) == GUILDERR_NOGUILD);
    CHECK(errorOf(dbDeclareGuildWar(1, "nosuchguild", now)) == GUILDERR_BADTARGET);
    CHECK(errorOf(dbDeclareGuildWar(1, to_cstr(u8"沙巴克"), now)) == GUILDERR_SELFWAR);
    CHECK(!dbAtGuildWar(guildID, otherID, now));

    const auto enemyID = dbDeclareGuildWar(1, to_cstr(u8"另一个"), now);
    REQUIRE(enemyID.has_value());
    CHECK(enemyID.value() == otherID);

    CHECK(dbAtGuildWar(guildID, otherID, now));
    CHECK(dbAtGuildWar(otherID, guildID, now));
    CHECK(!dbAtGuildWar(guildID, guildID, now));
    CHECK(!dbAtGuildWar(guildID, 0, now));

    // both sides see it
    const auto warList = dbLoadGuildWarList(guildID, now);
    REQUIRE(warList.size() == 1);
    CHECK(warList[0].guildID == otherID);
    CHECK(warList[0].guildName == to_cstr(u8"另一个"));
    CHECK(warList[0].endTime == now + SYS_GUILDWARTIME);

    const auto otherWarList = dbLoadGuildWarList(otherID, now);
    REQUIRE(otherWarList.size() == 1);
    CHECK(otherWarList[0].guildID == guildID);
    CHECK(otherWarList[0].guildName == to_cstr(u8"沙巴克"));

    // it ends by itself
    const auto endTime = now + SYS_GUILDWARTIME;
    CHECK( dbAtGuildWar(guildID, otherID, endTime - 1));
    CHECK(!dbAtGuildWar(guildID, otherID, endTime));
    CHECK(dbLoadGuildWarList(guildID, endTime).empty());

    // declaring again restarts it, from either side
    REQUIRE(dbDeclareGuildWar(3, to_cstr(u8"沙巴克"), endTime + 10).has_value());
    CHECK(dbAtGuildWar(guildID, otherID, endTime + 10));
    CHECK(dbLoadGuildWarList(guildID, endTime + 10).at(0).endTime == endTime + 10 + SYS_GUILDWARTIME);
    CHECK(countRows("tbl_guildwar") == 1);

    // a third guild has its own wars
    const auto thirdID = createGuild(4, u8"第三个");
    REQUIRE(dbDeclareGuildWar(4, to_cstr(u8"沙巴克"), endTime + 20).has_value());
    CHECK(dbLoadGuildWarList(guildID, endTime + 20).size() == 2);
    CHECK(dbLoadGuildWarList(otherID, endTime + 20).size() == 1);
    CHECK(!dbAtGuildWar(otherID, thirdID, endTime + 20));

    // and a war dies with its guild
    REQUIRE(dbDisbandGuild(1).has_value());
    CHECK(countRows("tbl_guildwar") == 0);
    CHECK(dbLoadGuildWarList(otherID, endTime + 20).empty());
}
