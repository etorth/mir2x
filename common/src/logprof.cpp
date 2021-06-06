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

#include <string>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include "strf.hpp"
#include "totype.hpp"
#include "logprof.hpp"
#include "raiitimer.hpp"

void logDisableProfiler()
{
    _logProf::logEnableProfiler = false;
}

void logProfiling(const std::function<void(const std::string &)> &dump)
{
	if(!dump){
		return;
	}

	if(!_logProf::logEnableProfiler){
		return;
	}

	const auto binaryName = []() -> std::string
	{
#ifdef _GNU_SOURCE
		return std::string(program_invocation_name);
#endif
		std::string line;
		std::ifstream f("/proc/self/cmdline");
		if(f && std::getline(f, line)){
			return std::string(line.c_str());
		}
		throw std::runtime_error("can not access /proc/self/cmdline");
	}();

#define _FUNC_DUMP_OP(op, ...) op(str_printf(__VA_ARGS__))

    _FUNC_DUMP_OP(dump, "---\n");
    _FUNC_DUMP_OP(dump, "---\n");
    _FUNC_DUMP_OP(dump, "--- Binary %s, pid = %lld, dbg_profiler runtime statistics:\n", binaryName.c_str(), to_lld(getpid()));
    _FUNC_DUMP_OP(dump, "--- \n");
    _FUNC_DUMP_OP(dump, "--- ---------------------------- ---------------- ---------------- ----------------\n");
    _FUNC_DUMP_OP(dump, "--- Command Name                            Calls     Longest Time       Total Time\n");
    _FUNC_DUMP_OP(dump, "--- ---------------------------- ---------------- ---------------- ----------------\n");

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

    for(const auto &profEntry: _logProf::logProfilerEntrySink){
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
		_FUNC_DUMP_OP(dump, "--- %-28s %16lld %11.3f secs %11.3f secs\n", entry.name, entry.count, entry.longest / 1000000000.0f, entry.total / 1000000000.0f);
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

	_FUNC_DUMP_OP(dump, "---\n");
	_FUNC_DUMP_OP(dump, "--- Profiler ran %lld times, average %.3f nsec/run.\n", totalCount, profilerAvgTime);
	_FUNC_DUMP_OP(dump, "---\n");
	_FUNC_DUMP_OP(dump, "---\n");

#undef _FUNC_DUMP_OP
}

bool _logProf::logEnableProfiler = true;
std::array<_logProf::logProfilerEntry, _logProf::maxProfilerCount()> _logProf::logProfilerEntrySink;
