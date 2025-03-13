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

    public:
        uint64_t to_nsec() const;

    public:
        uint64_t to_usec() const
        {
            return to_nsec() / 1000ULL;
        }

        uint64_t to_msec() const
        {
            return to_nsec() / 1000000ULL;
        }

        uint64_t to_sec () const
        {
            return to_nsec() / 1000000000ULL;
        }

    public:
        double to_secf() const
        {
            return static_cast<double>(to_nsec()) / 1000000000.0;
        }

    public:
        static uint64_t localtime();
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

        double diff_nsecf() const
        {
            return static_cast<double>(diff_nsec());
        }

    public:
        uint64_t diff_usec() const { return diff_nsec() / 1000ULL; }
        uint64_t diff_msec() const { return diff_nsec() / 1000000ULL; }
        uint64_t diff_sec () const { return diff_nsec() / 1000000000ULL; }

    public:
        double diff_usecf() const { return diff_nsecf() / 1000.0; }
        double diff_msecf() const { return diff_nsecf() / 1000000.0; }
        double diff_secf () const { return diff_nsecf() / 1000000000.0; }

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
