/*
 * =====================================================================================
 *
 *       Filename: raiitimer.cpp
 *        Created: 12/05/2018 01:42:56
 *    Description: high resolution timer for profiling
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
        throw fflerror("clock_gettime() failed");
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

uint64_t hres_tstamp::to_usec() const
{
    return to_nsec() / 1000ULL;
}

uint64_t hres_tstamp::to_msec() const
{
    return to_nsec() / 1000000ULL;
}
