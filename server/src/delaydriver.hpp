#pragma once
#include <cstdint>
#include <thread>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>
#include <unordered_map>
#include <asio.hpp>

class DelayDriver
{
    private:
        using timer_map  = std::unordered_map<uint64_t, asio::steady_timer>;
        using timer_node = std::unordered_map<uint64_t, asio::steady_timer>::node_type;

    private:
        std::unique_ptr<asio::io_context> m_context;

    private:
        uint64_t m_seqID = 1;

    private:
        std::mutex m_lock;
        std::thread m_thread;

    private:
        timer_map m_timerList;
        std::vector<timer_node> m_nodeList;

    public:
        DelayDriver();

    public:
        ~DelayDriver();

    public:
        static bool isDelayThread();

    public:
        uint64_t add(const std::pair<uint64_t, uint64_t> &, uint64_t);
        bool remove(uint64_t);

    private:
        void recycleTimer(uint64_t);
};
