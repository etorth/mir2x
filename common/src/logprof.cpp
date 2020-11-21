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

void logProfiling()
{
    if(fvProf::g_logEnableProfiler){
        logInfo(67289, "---\n");
        logInfo(67289, "---\n");
        logInfo(67289, "--- PID = %lld, runtime statistics:\n", FV_LLD(getpid()));
        logInfo(67289, "--- \n");
        logInfo(67289, "--- ---------------------------- ---------------- ---------------- ----------------\n");
        logInfo(67289, "--- Command Name                            Calls     Longest Time       Total Time\n");
        logInfo(67289, "--- ---------------------------- ---------------- ---------------- ----------------\n");

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

        for(const auto &profEntry: fvProf::g_logProfilerEntrySink){
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

            if(std::strcmp(entry.name, __func__) == 0){
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
            logInfo(67289, "--- %-28s %16lld %11.3f secs %11.3f secs\n", entry.name, entry.count, entry.longest / 1000000000.0f, entry.total / 1000000000.0f);
        }

        const auto profilerAvgTime = []() -> float
        {
            constexpr long profilerCount = 99999;
            const auto startTime = fvProf::getCurrTick();

            for(long i = 0; i < profilerCount; ++i){
                logProfiler();
            }
            return 1.0f * (fvProf::getCurrTick() - startTime) / profilerCount;
        }();

        logInfo(67289, "---\n");
        logInfo(67289, "--- Profiler ran %lld times, average %.3f nsec/run.\n", totalCount, profilerAvgTime);
        logInfo(67289, "---\n");
        logInfo(67289, "---\n");
    }
}

void logEnd()
{
    g_logPtr.reset();
}

std::string logNowStr()
{
    const auto sys_now = std::chrono::system_clock::now();
    const auto in_time_t = std::chrono::system_clock::to_time_t(sys_now);

    struct tm now_time;
    std::stringstream ss;

    ss << std::put_time(localtime_r(&in_time_t, &now_time), "%Y-%m-%d %X");
    return ss.str();
}

logAutoDebug::logAutoDebug(const char *fmt, ...)
{
    if(!g_logPtr->debugEnabled()){
        return;
    }

    va_list ap;
    va_start(ap, fmt);

    m_log = str_vprintf(fmt, ap);
    va_end(ap);

    logDebug(52419, "begin: %s", m_log.c_str());
}

logAutoDebug::~logAutoDebug()
{
    if(!g_logPtr->debugEnabled()){
        return;
    }
    logDebug(52419, "end  : %s", m_log.c_str());
}

logAutoTrace::logAutoTrace(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    m_log = str_vprintf(fmt, ap);
    va_end(ap);

    logTrace(52420, "begin: %s", m_log.c_str());
}

logAutoTrace::~logAutoTrace()
{
    logTrace(52420, "end  : %s", m_log.c_str());
}

const bool fvProf::g_logEnableProfiler = []()
{
    return std::getenv("FV_DCC_UPLOAD_DISABLE_PROFILER") == nullptr;
}();
std::array<fvProf::logProfilerEntry, fvProf::maxProfilerCount()> fvProf::g_logProfilerEntrySink;
