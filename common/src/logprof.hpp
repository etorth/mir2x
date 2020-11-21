/*
 * =====================================================================================
 *
 *       Filename: logprof.hpp
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

#pragma once
#include <string>
#include <type_traits>
#include "qt.h"

namespace fvProf
{
    // include all file names here if you want to use the macro logProfiler() inside
    // every time recompile all source files, not recommended for other modules except fvDCCUpload

    constexpr const char *fileNameList[]
    {
        "fvDCCUpload.h",
        "fvDCCUpload.C",
        "fvDCCUploadLogs.h",
        "fvDCCUploadLogs.C",
        "fvDCCUploadUtils.h",
        "fvDCCUploadUtils.C",
    };

    constexpr size_t strLiteralLength(const char *s)
    {
        size_t length = 0;
        while(s[length] != '\0'){
            length++;
        }
        return length;
    }

    template<size_t N> constexpr bool strLiteralRMatch(const char *s1, const char (&s2)[N])
    {
        int p1 = (int)(strLiteralLength(s1)) - 1;
        int p2 = (int)(N) - 2;

        while(p1 >= 0 && p2 >= 0){
            if(s1[p1] != s2[p2]){
                return false;
            }
            p1--;
            p2--;
        }
        return true;
    }

    template<size_t N> constexpr size_t fileName2Index(const char (&str)[N])
    {
        for(size_t i = 0; i < std::extent<decltype(fileNameList)>::value; ++i){
            if(fvProf::strLiteralRMatch(fileNameList[i], str)){
                return i;
            }
        }
        return SIZE_MAX;
    }

    template<size_t fileIndex, int counter> constexpr size_t loc2Hash()
    {
        static_assert(fileIndex < std::extent<decltype(fileNameList)>::value, "Put filename to fvDCCUploadLogs.h fileNameList[] table");
        static_assert(counter   < 1024, "too much markers in single file, limited by 1024");
        return fileIndex * 1024 + counter;
    }

    constexpr size_t maxProfilerCount()
    {
        return std::extent<decltype(fileNameList)>::value * 1024;
    }

    inline long long getCurrTick()
    {
        timespec now;
        clock_gettime(CLOCK_REALTIME, &now);
        return ((long long)(now.tv_sec) * 1000000000LL + (long long)(now.tv_nsec));
    }

    struct logProfilerEntry
    {
        std::atomic<long long> count   {0};
        std::atomic<long long> time    {0};
        std::atomic<long long> maxtime {0};
        std::atomic<const char *> name {nullptr};
    };

    extern const bool g_logEnableProfiler;
    extern std::array<logProfilerEntry, fvProf::maxProfilerCount()> g_logProfilerEntrySink;

    class logProfilerHelper
    {
        private:
            size_t m_loc = 0;
            const char *m_funcName = nullptr;

        private:
            long long m_startTime = 0;

        public:
            logProfilerHelper(size_t loc, const char *funcName)
            {
                if(g_logEnableProfiler){
                    m_loc = loc;
                    m_funcName = funcName;
                    m_startTime = fvProf::getCurrTick();
                }
            }

            ~logProfilerHelper()
            {
                if(!g_logEnableProfiler){
                    return;
                }

                auto &sinkRef = g_logProfilerEntrySink.at(m_loc);
                sinkRef.count.fetch_add(1, std::memory_order_relaxed);
                sinkRef.name.store(m_funcName, std::memory_order_relaxed);

                const long long nsec = fvProf::getCurrTick() - m_startTime;
                sinkRef.time.fetch_add(nsec, std::memory_order_relaxed);

                // need stronger memory order
                // https://herbsutter.com/2012/08/31/reader-qa-how-to-write-a-cas-loop-using-stdatomics/

                long long prev = sinkRef.maxtime.load();
                while(prev < nsec && !sinkRef.maxtime.compare_exchange_weak(prev, nsec));
            }
    };
}

#define logProfilerHelper_name(counter) logProfilerHelper_inst_##counter
#define logProfilerHelper_inst(counter) logProfilerHelper_name(counter)
#define logProfiler()           fvProf::logProfilerHelper logProfilerHelper_inst(__LINE__) {fvProf::loc2Hash<fvProf::fileName2Index(__FILE__), __COUNTER__>(), __func__}
#define logScopedProfiler(name) fvProf::logProfilerHelper logProfilerHelper_inst(__LINE__) {fvProf::loc2Hash<fvProf::fileName2Index(__FILE__), __COUNTER__>(), "" name }

#define logNamedProfilerHelper_line(linenum) #linenum
#define logNamedProfilerHelper_lstr(linenum) logNamedProfilerHelper_line(linenum)
#define logNamedProfilerHelper_name(counter) logNamedProfilerHelper_inst_##counter
#define logNamedProfilerHelper_inst(counter) logNamedProfilerHelper_name(counter)
#define logNamedProfiler(name) fvProf::logProfilerHelper logNamedProfilerHelper_inst(__LINE__) {fvProf::loc2Hash<fvProf::fileName2Index(__FILE__), __COUNTER__>(), "" name "_line_" logNamedProfilerHelper_lstr(__LINE__)}
