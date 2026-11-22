#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <expected>
#include <optional>
#include "raiitimer.hpp"
#include "serdesmsg.hpp"

// database is the source of truth, a player actor only caches its own membership
// another actor can change it directly, e.g. the leader kicks a member who is offline
//
// functions taking two dbids take the acting player first, the leader is checked in the database

struct DBGuildMembership
{
    uint32_t guildID = 0;
    std::string guildName {};

    int rank = 0;
    std::string title {};

    bool operator == (const DBGuildMembership &) const = default;
};

bool guildNameValid  (const std::string &);
bool guildTitleValid (const std::string &);
bool guildNoticeValid(const std::string &);

std::optional<DBGuildMembership> dbLoadGuildMembership(uint32_t);
uint32_t dbLoadGuildID(uint32_t); // 0 if not in a guild

SDGuildInfo dbLoadGuildInfo(uint32_t); // members' online flag is left false
std::vector<uint32_t> dbLoadGuildMemberList(uint32_t);

uint32_t dbFindCharDBID(const std::string &); // 0 if no such char

std::expected<uint32_t, int> dbCreateGuild(uint32_t, const std::string &);

int dbInviteGuildMember(uint32_t, uint32_t);
int dbAcceptGuildInvite(uint32_t, uint32_t); // invited dbid, guild id
std::optional<uint32_t> dbRejectGuildInvite(uint32_t, uint32_t); // the inviter, nullopt if no such invite

int dbLeaveGuild         (uint32_t);
int dbKickGuildMember    (uint32_t, uint32_t);
int dbTransferGuildLeader(uint32_t, uint32_t);

int dbSetGuildNotice     (uint32_t, const std::string &);
int dbSetGuildMemberTitle(uint32_t, uint32_t, const std::string &);

std::expected<std::vector<uint32_t>, int> dbDisbandGuild(uint32_t); // dbids of the other members

// declaring again restarts the war, takes the leader's dbid and the enemy guild name, returns the enemy guild id
std::expected<uint32_t, int> dbDeclareGuildWar(uint32_t, const std::string &, uint64_t = hres_tstamp::epoch());

bool dbAtGuildWar(uint32_t, uint32_t, uint64_t = hres_tstamp::epoch()); // two guild ids
std::vector<SDGuildWar> dbLoadGuildWarList(uint32_t, uint64_t = hres_tstamp::epoch()); // ordered by end time
