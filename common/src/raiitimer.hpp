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

class hres_tstamp
{
    private:
#ifdef _MSC_VER
        LARGE_INTEGER m_tstamp;
#else
        struct timespec m_tstamp;
#endif
    public:
        hres_tstamp();
        uint64_t to_nsec() const;
};

class hres_timer
{
    private:
        hres_tstamp m_start;

    public:
        hres_timer()
            : m_start()
        {}

    public:
        void reset()
        {
            m_start = hres_tstamp();
        }

    public:
        uint64_t diff_nsec() const
        {
            return hres_tstamp().to_nsec() - m_start.to_nsec();
        }

    public:
        uint64_t diff_usec() const { return diff_nsec() / 1000ULL; }
        uint64_t diff_msec() const { return diff_nsec() / 1000000ULL; }
        uint64_t diff_sec () const { return diff_nsec() / 1000000000ULL; }

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
