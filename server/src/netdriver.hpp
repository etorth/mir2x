#pragma once
#include <asio.hpp>
#include <cstdint>
#include <array>
#include <tuple>
#include <atomic>
#include <thread>
#include <queue>
#include <cstdint>
#include "uidf.hpp"
#include "channel.hpp"
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "dispatcher.hpp"
#include "actormsgpack.hpp"

class Channel;
class NetDriver final
{
    private:
        friend class Channel;

    private:
        struct ChannelSlot
        {
            // lock-protected buf
            // actor thread and asio thread access it
            std::mutex lock;
            std::vector<uint8_t> sendBuf;

            // sctatch buffer, accessed by actor thread only
            // used when post messages that need xor compression
            std::vector<uint8_t> encodeBuf;

            // channel
            // only asio thread can access it
            // actor thread access it through asio::post(access_handler)
            std::shared_ptr<Channel> channPtr;
        };

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
        std::vector<ChannelSlot> m_channList;

    public:
        NetDriver();

    public:
        ~NetDriver();

    public:
        static bool isNetThread();

    public:
        void launch(uint32_t);

    public:
        // these functions are provided to actor thread
        // actor thread send/receive message by these interfaces
        // actor thread can invalidate channel by calling close(channID), asio loop can invalidate by catching exception
        // but no method to check channel is valid, after actor thread invalidate it, actor thread should keep flag to prevent access an invalidated channel
        void close(uint32_t);                                   // request a channel to be closed
        void bindPlayer(uint32_t, uint64_t);                    // request a channel to forward all net message to an UID
        void post(uint32_t, uint8_t, const void *, size_t);     // post message to a channel

    private:
        void acceptNewConnection();

    private:
        void doRelease();
        void doClose(uint32_t);

    private:
        std::array<std::tuple<const uint8_t *, size_t>, 2> encodePostBuf(uint8_t, const void *, size_t, std::vector<uint8_t> &);
};
