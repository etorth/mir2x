#include <algorithm>
#include <string_view>
#include "dbpod.hpp"
#include "utf8f.hpp"
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "raiitimer.hpp"
#include "protocoldef.hpp"
#include "guilddb.hpp"

extern DBPod *g_dbPod;

static bool validDBID(uint32_t dbid)
{
    return dbid > 0 && dbid <= SYS_MAXDBID;
}

static const char *defaultGuildTitle(int rank)
{
    switch(rank){
        case GUILDRANK_LEADER: return to_cstr(u8"掌门人");
        case GUILDRANK_MEMBER: return to_cstr(u8"成员");
        default: throw fflvalue(rank);
    }
}

static void dbInsertGuildMember(uint32_t guildID, uint32_t dbid, int rank)
{
    auto query = g_dbPod->createQuery(
        u8R"###( insert into tbl_guildmember(fld_dbid, fld_guild, fld_rank, fld_title, fld_jointime) )###"
        u8R"###( values                                                                              )###"
        u8R"###(     (%llu, %llu, %d, ?, %llu)                                                       )###",

        to_llu(dbid),
        to_llu(guildID),
        rank,
        to_llu(hres_tstamp::epoch()));

    query.bind(1, defaultGuildTitle(rank));
    query.exec();
}

static void dbSetGuildRank(uint32_t dbid, int rank)
{
    auto query = g_dbPod->createQuery("update tbl_guildmember set fld_rank = %d, fld_title = ? where fld_dbid = %llu", rank, to_llu(dbid));
    query.bind(1, defaultGuildTitle(rank));
    fflassert(query.exec() == 1, dbid, rank);
}

// id of the guild dbid leads
static std::expected<uint32_t, int> dbLoadLedGuildID(uint32_t dbid)
{
    fflassert(validDBID(dbid), dbid);
    auto query = g_dbPod->createQuery("select fld_guild, fld_rank from tbl_guildmember where fld_dbid = %llu", to_llu(dbid));

    if(!query.executeStep()){
        return std::unexpected(GUILDERR_NOGUILD);
    }

    if(query.getColumn("fld_rank").getInt() != GUILDRANK_LEADER){
        return std::unexpected(GUILDERR_NOTLEADER);
    }
    return check_cast<uint32_t, unsigned>(query.getColumn("fld_guild"));
}

bool guildNameValid(const std::string &name)
{
    if(name.empty() || name.size() >= SYS_GUILDNAMESIZE || !utf8f::valid(name)){
        return false;
    }

    // name goes into xml layouts of NPC dialogs as is
    return std::ranges::none_of(name, [](char ch)
    {
        const auto byte = static_cast<unsigned char>(ch);
        return byte <= 0X20 || byte == 0X7F || std::string_view("<>&\"'").contains(ch);
    });
}

std::optional<DBGuildMembership> dbLoadGuildMembership(uint32_t dbid)
{
    fflassert(validDBID(dbid), dbid);
    auto query = g_dbPod->createQuery(
        u8R"###( select                                                                  )###"
        u8R"###(     tbl_guild.fld_id, tbl_guild.fld_name,                               )###"
        u8R"###(     tbl_guildmember.fld_rank, tbl_guildmember.fld_title                 )###"
        u8R"###( from                                                                    )###"
        u8R"###(     tbl_guildmember join tbl_guild                                      )###"
        u8R"###(     on                                                                  )###"
        u8R"###(         tbl_guild.fld_id = tbl_guildmember.fld_guild                    )###"
        u8R"###( where                                                                   )###"
        u8R"###(     tbl_guildmember.fld_dbid = %llu                                     )###", to_llu(dbid));

    if(!query.executeStep()){
        return std::nullopt;
    }

    return DBGuildMembership
    {
        .guildID = check_cast<uint32_t, unsigned>(query.getColumn("fld_id")),
        .guildName = query.getColumn("fld_name").getString(),

        .rank = query.getColumn("fld_rank").getInt(),
        .title = query.getColumn("fld_title").getString(),
    };
}

uint32_t dbLoadGuildID(uint32_t dbid)
{
    fflassert(validDBID(dbid), dbid);
    auto query = g_dbPod->createQuery("select fld_guild from tbl_guildmember where fld_dbid = %llu", to_llu(dbid));

    if(query.executeStep()){
        return check_cast<uint32_t, unsigned>(query.getColumn("fld_guild"));
    }
    return 0;
}

std::expected<uint32_t, int> dbCreateGuild(uint32_t dbid, const std::string &guildName)
{
    fflassert(validDBID(dbid), dbid);
    if(!guildNameValid(guildName)){
        return std::unexpected(GUILDERR_BADNAME);
    }

    auto transaction = g_dbPod->createTransaction();
    if(dbLoadGuildID(dbid)){
        return std::unexpected(GUILDERR_INGUILD);
    }

    {
        auto query = g_dbPod->createQuery("select 1 from tbl_guild where fld_name = ?");
        query.bind(1, guildName);

        if(query.executeStep()){
            return std::unexpected(GUILDERR_NAMETAKEN);
        }
    }

    auto insertQuery = g_dbPod->createQuery("insert into tbl_guild(fld_name, fld_createtime) values (?, %llu) returning fld_id", to_llu(hres_tstamp::epoch()));
    insertQuery.bind(1, guildName);

    fflassert(insertQuery.executeStep());
    const auto guildID = check_cast<uint32_t, unsigned>(insertQuery.getColumn("fld_id"));
    fflassert(!insertQuery.executeStep());

    dbInsertGuildMember(guildID, dbid, GUILDRANK_LEADER);
    transaction.commit();
    return guildID;
}

int dbAddGuildMember(uint32_t leaderDBID, uint32_t dbid)
{
    if(!validDBID(dbid) || dbid == leaderDBID){
        return GUILDERR_BADTARGET;
    }

    auto transaction = g_dbPod->createTransaction();
    const auto guildID = dbLoadLedGuildID(leaderDBID);

    if(!guildID.has_value()){
        return guildID.error();
    }

    if(dbLoadGuildID(dbid)){
        return GUILDERR_TARGETINGUILD;
    }

    dbInsertGuildMember(guildID.value(), dbid, GUILDRANK_MEMBER);
    transaction.commit();
    return GUILDERR_NONE;
}

int dbLeaveGuild(uint32_t dbid)
{
    fflassert(validDBID(dbid), dbid);
    auto transaction = g_dbPod->createTransaction();

    const auto guild = dbLoadGuildMembership(dbid);
    if(!guild.has_value()){
        return GUILDERR_NOGUILD;
    }

    if(guild->rank == GUILDRANK_LEADER){
        return GUILDERR_LEADERLEAVE;
    }

    g_dbPod->exec("delete from tbl_guildmember where fld_dbid = %llu", to_llu(dbid));
    transaction.commit();
    return GUILDERR_NONE;
}

int dbKickGuildMember(uint32_t leaderDBID, uint32_t dbid)
{
    if(!validDBID(dbid) || dbid == leaderDBID){
        return GUILDERR_BADTARGET;
    }

    auto transaction = g_dbPod->createTransaction();
    const auto guildID = dbLoadLedGuildID(leaderDBID);

    if(!guildID.has_value()){
        return guildID.error();
    }

    if(g_dbPod->exec("delete from tbl_guildmember where fld_dbid = %llu and fld_guild = %llu", to_llu(dbid), to_llu(guildID.value())) == 0){
        return GUILDERR_NOTMEMBER;
    }

    transaction.commit();
    return GUILDERR_NONE;
}

int dbTransferGuildLeader(uint32_t leaderDBID, uint32_t dbid)
{
    if(!validDBID(dbid) || dbid == leaderDBID){
        return GUILDERR_BADTARGET;
    }

    auto transaction = g_dbPod->createTransaction();
    const auto guildID = dbLoadLedGuildID(leaderDBID);

    if(!guildID.has_value()){
        return guildID.error();
    }

    if(dbLoadGuildID(dbid) != guildID.value()){
        return GUILDERR_NOTMEMBER;
    }

    // demote first, the guild can have only one leader at any time
    dbSetGuildRank(leaderDBID, GUILDRANK_MEMBER);
    dbSetGuildRank(dbid, GUILDRANK_LEADER);

    transaction.commit();
    return GUILDERR_NONE;
}

std::expected<std::vector<uint32_t>, int> dbDisbandGuild(uint32_t leaderDBID)
{
    auto transaction = g_dbPod->createTransaction();
    const auto guildID = dbLoadLedGuildID(leaderDBID);

    if(!guildID.has_value()){
        return std::unexpected(guildID.error());
    }

    std::vector<uint32_t> memberList;
    {
        auto query = g_dbPod->createQuery("select fld_dbid from tbl_guildmember where fld_guild = %llu and fld_dbid != %llu", to_llu(guildID.value()), to_llu(leaderDBID));
        while(query.executeStep()){
            memberList.push_back(check_cast<uint32_t, unsigned>(query.getColumn("fld_dbid")));
        }
    }

    g_dbPod->exec("delete from tbl_guild where fld_id = %llu", to_llu(guildID.value()));
    transaction.commit();
    return memberList;
}
