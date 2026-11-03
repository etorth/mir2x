#include <format>
#include <algorithm>
#include "totype.hpp"
#include "teamrequirement.hpp"

bool TeamRequirement::anyJobAllowed() const
{
    return std::ranges::any_of(jobList, [](const auto &jobReq){ return jobReq.allow; });
}

std::optional<size_t> TeamRequirement::jobIndex(int job)
{
    for(size_t i = 0; i < jobInfoList.size(); ++i){
        if(jobInfoList[i].job & job){
            return i;
        }
    }
    return std::nullopt;
}

bool TeamRequirement::accept(int job, int level, int stat, const std::function<bool(uint32_t)> &hasMagic) const
{
    const auto index = jobIndex(job);
    if(!index.has_value()){
        return false;
    }

    const auto &jobReq = jobList.at(index.value());
    if(!jobReq.allow){
        return false;
    }

    if(jobReq.minLevel > 0 && level < jobReq.minLevel){
        return false;
    }

    if(jobReq.maxLevel > 0 && level > jobReq.maxLevel){
        return false;
    }

    if(jobReq.minStat > 0 && stat < jobReq.minStat){
        return false;
    }

    return std::ranges::all_of(jobReq.magicIDList, [&hasMagic](uint32_t magicID){ return hasMagic && hasMagic(magicID); });
}

std::string TeamRequirement::summary() const
{
    if(std::ranges::all_of(jobList, [](const auto &jobReq){ return jobReq.allow && !jobReq.hasLimit(); })){
        return to_cstr(u8"无");
    }

    std::string result;
    for(size_t i = 0; i < jobList.size(); ++i){
        const auto &jobReq = jobList[i];
        if(!jobReq.allow){
            continue;
        }

        if(!result.empty()){
            result += " ";
        }

        result += to_cstr(jobInfoList[i].initial);
        if(jobReq.minLevel > 0 && jobReq.maxLevel > 0){
            result += std::format("{}-{}", jobReq.minLevel, jobReq.maxLevel);
        }
        else if(jobReq.minLevel > 0){
            result += std::format("{}+", jobReq.minLevel);
        }
        else if(jobReq.maxLevel > 0){
            result += std::format("{}-", jobReq.maxLevel);
        }

        if(jobReq.minStat > 0 || !jobReq.magicIDList.empty()){
            result += "*";
        }
    }
    return result;
}
