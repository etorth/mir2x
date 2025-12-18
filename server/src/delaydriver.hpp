#pragma once
#include <cstdint>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <map>
#include <unordered_map>
#include <optional>
#include <utility>
#include <vector>
#include <chrono>
#include "acnodewrapper.hpp"

class DelayDriver
{
    private:
        struct TimerEntry
        {
            uint64_t seqID;
            std::pair<uint64_t, uint64_t> cbArg;
        };

    private:
        using clock_type = std::chrono::steady_clock;

    private:
        using   waiter_map = std::unordered_map<uint64_t, std::pair<uint64_t, uint64_t>>;
        using    timer_map = std::multimap<clock_type::time_point, TimerEntry>;
        using timer_id_map = std::map<uint64_t, timer_map::iterator>;

    private:
        uint64_t m_seqID = 1;
        bool m_stopRequested = false;

    private:
        ACNodeWrapper<  waiter_map> m_waiters; // timers that never expire
        ACNodeWrapper<   timer_map> m_timers;
        ACNodeWrapper<timer_id_map> m_timerIds;

    private:
        std::vector<std::pair<uint64_t, uint64_t>> m_cancelledTimerArgs;

    private:
        std::mutex m_mutex;
        std::thread m_worker;
        std::condition_variable m_cond;

    public:
        DelayDriver();
        virtual ~DelayDriver();

    public:
        static bool isDelayThread();

    public:
        uint64_t add(const std::pair<uint64_t, uint64_t> &cbArg, uint64_t msec)
        {
            if(msec){ return addTimer (cbArg, msec); } // return odd
            else    { return addWaiter(cbArg      ); } // return even
        }

    public:
        std::optional<bool> cancel(uint64_t seqID, bool triggerCallbackInPlace = false)
        {
            if(seqID % 2){ return cancelTimer (seqID, triggerCallbackInPlace); } // odd
            else         { return cancelWaiter(seqID, triggerCallbackInPlace); } // even
        }

    public:
        void requestStop();

    private:
        void forceStop();
        void onTimerCallback(const std::pair<uint64_t, uint64_t> &, bool);

    private:
        uint64_t addTimer (const std::pair<uint64_t, uint64_t> &, uint64_t);
        uint64_t addWaiter(const std::pair<uint64_t, uint64_t> &);

    private:
        std::optional<bool> cancelTimer (uint64_t, bool);
        std::optional<bool> cancelWaiter(uint64_t, bool);
};
