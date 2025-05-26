#include "totype.hpp"
#include "fflerror.hpp"
#include "raiitimer.hpp"

// for QueryPerformanceFrequency(), the doc suggests to cache the result:
// https://docs.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancefrequency

#ifdef _MSC_VER
static const LARGE_INTEGER g_winFreq = []()
{
    LARGE_INTEGER result;
    QueryPerformanceFrequency(&result); // after winxp this function always succeed
    return result;
}();
#endif

hres_tstamp::hres_tstamp()
{
#ifdef _MSC_VER
    QueryPerformanceCounter(&m_tstamp); // after winxp this function always succeed
#else
    if(clock_gettime(CLOCK_MONOTONIC, &m_tstamp)) [[unlikely]] {
        throw fflerror("clock_gettime(CLOCK_MONOTONIC) failed");
    }
#endif
}

uint64_t hres_tstamp::to_nsec() const
{
#ifdef _MSC_VER
    return (to_u64(m_tstamp.QuadPart) * 1000000000ULL) / to_u64(g_winFreq.QuadPart);
#else
    return (to_u64(m_tstamp.tv_sec  ) * 1000000000ULL) + to_u64(m_tstamp.tv_nsec  );
#endif
}

uint64_t hres_tstamp::localtime()
{
#ifdef _MSC_VER
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
        throw fflerror("clock_gettime(CLOCK_REALTIME) failed");
    }

    struct tm buf;
    if(localtime_r(&ts.tv_sec, &buf) != &buf) [[unlikely]] {
        throw fflerror("localtime_r(%llu, %p) failed", to_llu(ts.tv_sec), to_cvptr(&buf));
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
    // using UINT64_MAX_MAX = 18446744073709551615, can use 19 digits since year now start from 2
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
