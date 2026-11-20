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

// guild names and titles go into xml layouts of NPC dialogs and system messages as is
static bool guildLabelValid(const std::string &label, size_t sizeLimit)
{
    if(label.empty() || label.size() >= sizeLimit || !utf8f::valid(label)){
        return false;
    }

    return std::ranges::none_of(label, [](char ch)
    {
        const auto byte = static_cast<unsigned char>(ch);
        return byte <= 0X20 || byte == 0X7F || std::string_view("<>&\"'").contains(ch);
    });
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
    return guildLabelValid(name, SYS_GUILDNAMESIZE);
}

bool guildTitleValid(const std::string &title)
{
    return guildLabelValid(title, SYS_GUILDTITLESIZE);
}

bool guildNoticeValid(const std::string &notice)
{
    if(notice.size() >= SYS_GUILDNOTICESIZE || !utf8f::valid(notice)){
        return false;
    }

    return std::ranges::none_of(notice, [](char ch)
    {
        const auto byte = static_cast<unsigned char>(ch);
        return (byte < 0X20 && ch != '\n') || byte == 0X7F;
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

SDGuildInfo dbLoadGuildInfo(uint32_t guildID)
{
    SDGuildInfo info {};
    {
        auto query = g_dbPod->createQuery("select fld_name, fld_notice from tbl_guild where fld_id = %llu", to_llu(guildID));
        if(!query.executeStep()){
            return info;
        }

        info.guildID = guildID;
        info.name    = query.getColumn("fld_name"  ).getString();
        info.notice  = query.getColumn("fld_notice").getString();
    }

    auto query = g_dbPod->createQuery(
        u8R"###( select                                                                          )###"
        u8R"###(     tbl_guildmember.fld_dbid, tbl_guildmember.fld_rank, tbl_guildmember.fld_title, )###"
        u8R"###(     tbl_char.fld_name                                                           )###"
        u8R"###( from                                                                            )###"
        u8R"###(     tbl_guildmember join tbl_char                                               )###"
        u8R"###(     on                                                                          )###"
        u8R"###(         tbl_char.fld_dbid = tbl_guildmember.fld_dbid                            )###"
        u8R"###( where                                                                           )###"
        u8R"###(     tbl_guildmember.fld_guild = %llu                                            )###"
        u8R"###( order by                                                                        )###"
        u8R"###(     tbl_guildmember.fld_rank, tbl_guildmember.fld_jointime, tbl_guildmember.fld_dbid )###", to_llu(guildID));

    while(query.executeStep()){
        info.memberList.push_back(SDGuildMember
        {
            .dbid = check_cast<uint32_t, unsigned>(query.getColumn("fld_dbid")),
            .name = query.getColumn("fld_name").getString(),

            .rank = query.getColumn("fld_rank").getInt(),
            .title = query.getColumn("fld_title").getString(),
        });
    }
    return info;
}

std::vector<uint32_t> dbLoadGuildMemberList(uint32_t guildID)
{
    std::vector<uint32_t> memberList;
    auto query = g_dbPod->createQuery("select fld_dbid from tbl_guildmember where fld_guild = %llu", to_llu(guildID));

    while(query.executeStep()){
        memberList.push_back(check_cast<uint32_t, unsigned>(query.getColumn("fld_dbid")));
    }
    return memberList;
}

uint32_t dbFindCharDBID(const std::string &charName)
{
    auto query = g_dbPod->createQuery("select fld_dbid from tbl_char where fld_name = ?");
    query.bind(1, charName);

    if(query.executeStep()){
        return check_cast<uint32_t, unsigned>(query.getColumn("fld_dbid"));
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
    g_dbPod->exec("delete from tbl_guildinvite where fld_dbid = %llu", to_llu(dbid));

    transaction.commit();
    return guildID;
}

int dbInviteGuildMember(uint32_t leaderDBID, uint32_t dbid)
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

    g_dbPod->exec(
        u8R"###( insert or replace into tbl_guildinvite(fld_dbid, fld_guild, fld_inviter, fld_invitetime) )###"
        u8R"###( values                                                                                    )###"
        u8R"###(     (%llu, %llu, %llu, %llu)                                                              )###",

        to_llu(dbid),
        to_llu(guildID.value()),
        to_llu(leaderDBID),
        to_llu(hres_tstamp::epoch()));

    transaction.commit();
    return GUILDERR_NONE;
}

int dbAcceptGuildInvite(uint32_t dbid, uint32_t guildID)
{
    fflassert(validDBID(dbid), dbid);
    auto transaction = g_dbPod->createTransaction();

    // a disbanded guild takes its invites down with it
    if(!g_dbPod->createQuery("select 1 from tbl_guildinvite where fld_dbid = %llu and fld_guild = %llu", to_llu(dbid), to_llu(guildID)).executeStep()){
        return GUILDERR_NOINVITE;
    }

    if(dbLoadGuildID(dbid)){
        return GUILDERR_INGUILD;
    }

    dbInsertGuildMember(guildID, dbid, GUILDRANK_MEMBER);
    g_dbPod->exec("delete from tbl_guildinvite where fld_dbid = %llu", to_llu(dbid));

    transaction.commit();
    return GUILDERR_NONE;
}

std::optional<uint32_t> dbRejectGuildInvite(uint32_t dbid, uint32_t guildID)
{
    fflassert(validDBID(dbid), dbid);
    auto query = g_dbPod->createQuery("delete from tbl_guildinvite where fld_dbid = %llu and fld_guild = %llu returning fld_inviter", to_llu(dbid), to_llu(guildID));

    if(!query.executeStep()){
        return std::nullopt;
    }

    const auto inviter = check_cast<uint32_t, unsigned>(query.getColumn("fld_inviter"));
    fflassert(!query.executeStep());
    return inviter;
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

int dbSetGuildNotice(uint32_t leaderDBID, const std::string &notice)
{
    if(!guildNoticeValid(notice)){
        return GUILDERR_BADNOTICE;
    }

    auto transaction = g_dbPod->createTransaction();
    const auto guildID = dbLoadLedGuildID(leaderDBID);

    if(!guildID.has_value()){
        return guildID.error();
    }

    auto query = g_dbPod->createQuery("update tbl_guild set fld_notice = ? where fld_id = %llu", to_llu(guildID.value()));
    query.bind(1, notice);
    query.exec();

    transaction.commit();
    return GUILDERR_NONE;
}

int dbSetGuildMemberTitle(uint32_t leaderDBID, uint32_t dbid, const std::string &title)
{
    if(!validDBID(dbid)){
        return GUILDERR_BADTARGET;
    }

    if(!guildTitleValid(title)){
        return GUILDERR_BADTITLE;
    }

    auto transaction = g_dbPod->createTransaction();
    const auto guildID = dbLoadLedGuildID(leaderDBID);

    if(!guildID.has_value()){
        return guildID.error();
    }

    auto query = g_dbPod->createQuery("update tbl_guildmember set fld_title = ? where fld_dbid = %llu and fld_guild = %llu", to_llu(dbid), to_llu(guildID.value()));
    query.bind(1, title);

    if(query.exec() == 0){
        return GUILDERR_NOTMEMBER;
    }

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

    auto memberList = dbLoadGuildMemberList(guildID.value());
    std::erase(memberList, leaderDBID);

    g_dbPod->exec("delete from tbl_guild where fld_id = %llu", to_llu(guildID.value()));
    transaction.commit();
    return memberList;
}
