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

class DelayDriver
{
    private:
        struct TimerEntry
        {
            uint64_t seqID;
            std::pair<uint64_t, uint64_t> cbArg;
        };

        using clock_type = std::chrono::steady_clock;

        using waiter_map  = std::unordered_map<uint64_t, std::pair<uint64_t, uint64_t>>;
        using waiter_node = waiter_map::node_type;

        using timer_map  = std::multimap<clock_type::time_point, TimerEntry>;
        using timer_node = timer_map::node_type;

        using timer_id_map  = std::map<uint64_t, timer_map::iterator>;
        using timer_id_node = timer_id_map::node_type;

    private:
        uint64_t m_seqID = 1;
        bool m_stopRequested = false;

    private:
        waiter_map m_waiters; // timers that never expire
        std::vector<waiter_node> m_waiterNodes;

        timer_map m_timers;
        std::vector<timer_node> m_timerNodes;

        timer_id_map m_timerIds;
        std::vector<timer_id_node> m_timerIdNodes;

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
