#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <expected>
#include <optional>

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

bool guildNameValid(const std::string &);

std::optional<DBGuildMembership> dbLoadGuildMembership(uint32_t);
uint32_t dbLoadGuildID(uint32_t); // 0 if not in a guild

std::expected<uint32_t, int> dbCreateGuild(uint32_t, const std::string &);

int dbAddGuildMember     (uint32_t, uint32_t);
int dbLeaveGuild         (uint32_t);
int dbKickGuildMember    (uint32_t, uint32_t);
int dbTransferGuildLeader(uint32_t, uint32_t);

std::expected<std::vector<uint32_t>, int> dbDisbandGuild(uint32_t); // dbids of the other members
