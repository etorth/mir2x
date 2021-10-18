#pragma once
#include <asio.hpp>
#include <atomic>
#include <thread>
#include <cstdint>
#include "uidf.hpp"
#include "channel.hpp"
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "monoserver.hpp"
#include "cachequeue.hpp"
#include "dispatcher.hpp"
#include "actormsgpack.hpp"

class Channel;
class NetDriver final
{
    private:
        friend class Channel;

    private:
        unsigned int m_port = 0;

    private:
        asio::io_service        *m_io       = nullptr;
        asio::ip::tcp::endpoint *m_endPoint = nullptr;
        asio::ip::tcp::acceptor *m_acceptor = nullptr;

    private:
        std::thread m_thread;

    private:
        std::queue<uint32_t> m_channIDQ;
        std::vector<std::shared_ptr<Channel>> m_channList;

    public:
        NetDriver();

    public:
        ~NetDriver();

    public:
        bool isNetThread() const;

    public:
        void launch(uint32_t);

    public:
        // these functions are provided to actor thread
        // actor thread send/receive message by these interfaces
        // actor thread can invalidate channel by calling close(channID), asio loop can invalidate by catching exception
        // but no method to check channel is valid, after actor thread invalidate it, actor thread should keep flag to prevent access an invalidated channel
        //
        // TODO thre is data race bug
        // when actor thread doing post but asio loop catches exception and calling doClose()
        // then still has issue, possible solution:
        // 1. use std::atomic<std::shared_ptr<Channel>>
        // 2. split channel into store/post/recive parts, store part always storing posting data from actor threads
        //    and post/receive parts are real asio related part
        void close(uint32_t);                                   // request a channel to be closed
        void bindPlayer(uint32_t, uint64_t);                    // request a channel to forward all net message to an UID
        void post(uint32_t, uint8_t, const void *, size_t);     // post message to a channel

    private:
        void acceptNewConnection();

    private:
        void doRelease();
        void doClose(uint32_t);
};
