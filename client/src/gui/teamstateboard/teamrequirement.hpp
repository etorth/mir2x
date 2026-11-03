#pragma once
#include <set>
#include <array>
#include <string>
#include <cstdint>
#include <optional>
#include <functional>
#include "protocoldef.hpp"

struct TeamJobRequirement final
{
    bool allow = true;

    int minLevel = 0; // 0 means no limit
    int maxLevel = 0;
    int minStat  = 0; // 战士: DC, 法师: MC, 道士: SC

    std::set<uint32_t> magicIDList {};

    bool hasLimit() const
    {
        return minLevel > 0 || maxLevel > 0 || minStat > 0 || !magicIDList.empty();
    }
};

struct TeamRequirement final
{
    struct JobInfo final
    {
        int job;

        const char8_t *name;
        const char8_t *initial;
        const char8_t *stat;
    };

    constexpr static std::array<JobInfo, 3> jobInfoList
    {{
        {JOB_WARRIOR, u8"战士", u8"战", u8"攻击"},
        {JOB_WIZARD , u8"法师", u8"法", u8"魔法"},
        {JOB_TAOIST , u8"道士", u8"道", u8"道术"},
    }};

    int maxMember = 0;
    bool autoAccept = false;

    std::array<TeamJobRequirement, jobInfoList.size()> jobList {};

    bool anyJobAllowed() const;
    bool accept(int, int, int, const std::function<bool(uint32_t)> &) const; // job, level, stat, hasMagic

    static std::optional<size_t> jobIndex(int);
    std::string summary() const; // short text for list column
};
