#if defined(__MINGW32__) || defined(__MINGW64__) || defined(_MSC_VER)
    #include <windows.h>
#else
    #include <time.h>
#endif

#include "totype.hpp"
#include "fflerror.hpp"
#include "raiitimer.hpp"

// for QueryPerformanceFrequency(), the doc suggests to cache the result:
// https://docs.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancefrequency

#if defined(__MINGW32__) || defined(__MINGW64__) || defined(_MSC_VER)
static const LARGE_INTEGER g_winFreq = []()
{
    LARGE_INTEGER result;
    QueryPerformanceFrequency(&result); // after winxp this function always succeed
    return result;
}();
#endif

hres_tstamp::hres_tstamp()
{
#if defined(__MINGW32__) || defined(__MINGW64__) || defined(_MSC_VER)
    LARGE_INTEGER tstamp;
    QueryPerformanceCounter(&tstamp); // after winxp this function always succeed
    m_tstamp = (to_u64(tstamp.QuadPart) * 1000000000ULL) / to_u64(g_winFreq.QuadPart);
#else
    struct timespec tstamp;
    if(clock_gettime(CLOCK_MONOTONIC, &tstamp)) [[unlikely]] {
        throw fflpanic("clock_gettime(CLOCK_MONOTONIC) failed");
    }
    m_tstamp = (to_u64(tstamp.tv_sec) * 1000000000ULL) + to_u64(tstamp.tv_nsec);
#endif
}

uint64_t hres_tstamp::localtime()
{
#if defined(__MINGW32__) || defined(__MINGW64__) || defined(_MSC_VER)
    SYSTEMTIME sys;
    GetLocalTime(&sys);

    const uint64_t year        = sys.wYear;
    const uint64_t month       = sys.wMonth;
    const uint64_t day         = sys.wDay;
    const uint64_t hour        = sys.wHour;
    const uint64_t minute      = sys.wMinute;
    const uint64_t second      = sys.wSecond;
    const uint64_t millisecond = sys.wMilliseconds;
#else
    struct timespec ts;
    if(clock_gettime(CLOCK_REALTIME, &ts)) [[unlikely]] {
        throw fflpanic("clock_gettime(CLOCK_REALTIME) failed");
    }

    struct tm buf;
    if(localtime_r(&ts.tv_sec, &buf) != &buf) [[unlikely]] {
        throw fflpanic("localtime_r({}, {:p}) failed", ts.tv_sec, to_cvptr(&buf));
    }

    const uint64_t year        = buf.tm_year + 1900;
    const uint64_t month       = buf.tm_mon  +    1;
    const uint64_t day         = buf.tm_mday;
    const uint64_t hour        = buf.tm_hour;
    const uint64_t minute      = buf.tm_min; // doc shows it's [0, 60], can be 60
    const uint64_t second      = buf.tm_sec;
    const uint64_t millisecond = ts.tv_nsec / UINT64_C(1000000);
#endif
    // return format
    // using UINT64_MAX = 18446744073709551615, can use 19 digits since year now start from 2
    //
    // year month day hour minute second millisecond, using 16 digits
    // 2022    11  23   08     22     59         123

    return year        * UINT64_C(10000000000000) +
           month       * UINT64_C(100000000000  ) +
           day         * UINT64_C(1000000000    ) +
           hour        * UINT64_C(10000000      ) +
           minute      * UINT64_C(100000        ) +
           second      * UINT64_C(1000          ) +
           millisecond * UINT64_C(1             ) ;
}
