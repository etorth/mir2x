/*
 * =====================================================================================
 *
 *       Filename: raiitimer.hpp
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

#pragma once
#ifdef _MSC_VER
    #include <windows.h>
#else
    #include "time.h"
#endif

#include <atomic>
#include <cstdint>

class hres_timer
{
#ifdef _MSC_VER
    private:
        LARGE_INTEGER m_start;
        LARGE_INTEGER m_freq;
#else
    private:
        struct timespec m_start;
#endif
    public:
        explicit hres_timer()
        {
#ifdef _MSC_VER
            // not sure how heavy is it
            // msdn document recommends to cache this value
            QueryPerformanceFrequency(&m_freq);
            if(m_freq.QuadPart == 0){
                // hr-counter not supported
                // after WinXP this error won't happen
            }
#endif
            reset();
        }

    public:
        ~hres_timer() = default;

    public:
        uint64_t diff_nsec() const
        {
#ifdef _MSC_VER
            LARGE_INTEGER curr_time;
            QueryPerformanceCounter(&curr_time);
            return ((uint64_t)(curr_time.QuadPart - m_start.QuadPart) * 1000000000ULL) / (uint64_t)(m_freq.QuadPart);
#else
            struct timespec curr_time;
            if(clock_gettime(CLOCK_MONOTONIC, &curr_time)){
                return 0;
            }

	    // 1. for 32bit machine (x.tv_sec - y.tv_sec) * 1000000000 can easily overflow
	    // 2. keep signed for d_sec * 1000000000 + d_nsec, then convert to unsignd

            constexpr long TIME_PRECISION = 1000000000;
            return (uint64_t)((int64_t)(curr_time.tv_sec - m_start.tv_sec) * TIME_PRECISION + (curr_time.tv_nsec - m_start.tv_nsec));
#endif
        }

        uint64_t diff_usec() const
        {
            return diff_nsec() / 1000;
        }

        uint64_t diff_msec() const
        {
            return diff_nsec() / 1000000;
        }

        uint64_t diff_sec() const
        {
            return diff_nsec() / 1000000000;
        }

    public:
        void reset()
        {
#ifdef _MSC_VER
            QueryPerformanceCounter(&m_start);
            if(m_start.QuadPart == 0){

            }
#else
            if(clock_gettime(CLOCK_MONOTONIC, &m_start)){
                // ...
            }
#endif
        }

    public:
        const auto &origin() const
        {
            return m_start;
        }
};

template<typename T = uint64_t> class raii_timer final: public hres_timer
{
    private:
        T *m_valptr;

    public:
        explicit raii_timer(T *valptr = nullptr)
            : hres_timer()
            , m_valptr(valptr)
        {
            static_assert(std::is_same<T, uint64_t>::value || std::is_same<T, std::atomic<uint64_t>>::value);
        }

    public:
        ~raii_timer()
        {
            if(m_valptr){
                *m_valptr += diff_nsec();
            }
        }

    public:
        void dismiss()
        {
            m_valptr = nullptr;
        }
};
