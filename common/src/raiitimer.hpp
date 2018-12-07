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
#include "time.h"
#include <atomic>
#include <cstdint>

class hres_timer
{
    private:
        struct timespec m_start;

    public:
        explicit hres_timer()
        {
            reset();
        }

    public:
        ~hres_timer() = default;

    public:
        uint64_t diff_nsec() const
        {
            struct timespec curr_time;
            if(clock_gettime(CLOCK_MONOTONIC, &curr_time)){
                return 0;
            }

            constexpr long TIME_PRECISION = 1000000000;
            return (uint64_t)((long)(curr_time.tv_sec - m_start.tv_sec) * TIME_PRECISION + (curr_time.tv_nsec - m_start.tv_nsec));
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
            if(clock_gettime(CLOCK_MONOTONIC, &m_start)){
                // ...
            }
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
        T *m_u64;

    public:
        explicit raii_timer(T *pu64 = nullptr)
            : hres_timer()
            , m_u64(pu64)
        {
            static_assert(std::is_same<T, uint64_t>::value || std::is_same<T, std::atomic<uint64_t>>::value);
        }

    public:
        ~raii_timer()
        {
            if(m_u64){
                *m_u64 += diff_nsec();
            }
        }

    public:
        void dismiss()
        {
            m_u64 = nullptr;
        }
};
