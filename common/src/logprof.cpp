/*
 * =====================================================================================
 *
 *       Filename: logprof.cpp
 *        Created: 11/20/2020 19:03:56
 *    Description:
 *
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include <cstring>
#include "strf.hpp"
#include "logprof.hpp"

void logDisableProfiler()
{
    _logProf::g_logEnableProfiler = false;
}

void logProfiling(const std::function<void(const std::string &)> &f)
{
    if(!f){
        return;
    }

    if(!_logProf::g_logEnableProfiler){
        return;
    }

    f("---\n");
    f("---\n");
    f("--- runtime statistics:\n");
    f("--- \n");
    f("--- ---------------------------- ---------------- ---------------- ----------------\n");
    f("--- Command Name                            Calls     Longest Time       Total Time\n");
    f("--- ---------------------------- ---------------- ---------------- ----------------\n");

    struct logEntry
    {
        const char *name = nullptr;
        long long count;
        long long total;
        long long longest;
    };

    long long totalTime = 0;
    long long totalCount = 0;
    std::vector<logEntry> logEntryList;

    for(const auto &profEntry: _logProf::g_logProfilerEntrySink){
        if(!profEntry.name.load()){
            continue;
        }

        const logEntry entry
        {
            profEntry.name.load(),
            profEntry.count.load(),
            profEntry.time.load(),
            profEntry.maxtime.load(),
        };

        if(std::strcmp(entry.name, "__test_log_profiler_speed_rand_728046896404976471527561404810") == 0){
            continue;
        }

        totalTime += entry.total;
        totalCount += entry.count;
        logEntryList.push_back(entry);
    }

    std::sort(logEntryList.begin(), logEntryList.end(), [](const auto &x, const auto &y) -> bool
    {
        return std::strcmp(x.name, y.name) < 0;
    });

    for(const auto &entry: logEntryList){
        f(str_printf("--- %-28s %16lld %11.3f secs %11.3f secs\n", entry.name, entry.count, entry.longest / 1000000000.0f, entry.total / 1000000000.0f));
    }

    const auto profilerAvgTime = []() -> float
    {
        constexpr long profilerCount = 9999;
        const auto startTime = _logProf::getCurrTick();

        for(long i = 0; i < profilerCount; ++i){
            logScopedProfiler("__test_log_profiler_speed_rand_728046896404976471527561404810");
        }
        return 1.0f * (_logProf::getCurrTick() - startTime) / profilerCount;
    }();

    f("---\n");
    f(str_printf("--- Profiler ran %lld times, average %.3f nsec/run.\n", totalCount, profilerAvgTime));
    f("---\n");
    f("---\n");
}

bool _logProf::g_logEnableProfiler = true;
std::array<_logProf::logProfilerEntry, _logProf::maxProfilerCount()> _logProf::g_logProfilerEntrySink;
