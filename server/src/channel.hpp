#pragma once
#include <asio.hpp>
#include <array>
#include <tuple>
#include <mutex>
#include <memory>
#include <vector>
#include <atomic>
#include <cstdint>
#include <stdexcept>
#include <functional>
#include "strf.hpp"
#include "dispatcher.hpp"

class ChannError: public std::exception
{
    private:
        const uint32_t m_channID;

    private:
        std::string m_what;

    public:
        ChannError(uint32_t channID)
            : m_channID(channID)
        {}

        ChannError(uint32_t channID, const char *format, ...)
            : m_channID(channID)
        {
            str_format(format, m_what);
        }

    public:
        const char *what() const noexcept override
        {
            return m_what.empty() ? "ChannError: unknown error" : m_what.c_str();
        }

    public:
        uint32_t channID() const
        {
            return m_channID;
        }
};

class Channel final: public std::enable_shared_from_this<Channel>
{
    private:
        enum ChannStateType: int
        {
            CS_NONE    = 0,
            CS_RUNNING = 1,
            CS_STOPPED = 2,
        };

    private:
        asio::ip::tcp::socket m_socket;

    private:
        const uint32_t m_id;

    private:
        std::atomic<int> m_state {CS_NONE};

    private:
        Dispatcher m_dispatcher;

    private:
        // for read channel packets
        // only asio main loop accesses these fields
        uint8_t  m_readHeadCode = 0;
        uint8_t  m_readLen[4]   = {0, 0, 0, 0};
        uint32_t m_bodyLen      = 0;

    private:
        std::vector<uint8_t> m_postBuf;
        std::vector<uint8_t> m_readBuf;
        std::vector<uint8_t> m_decodeBuf;

    private:
        uint64_t m_playerUID = 0;

    private:
        // for post channel packets
        // server thread and asio main loop accesses these feilds
        //
        // 1. m_flushFlag indicates there is procedure accessing m_currSendQ in asio main loop
        //    m_flushFlag prevents more than one procedure from accessing m_currSendQ
        //
        // 2. m_nextQLock protect the pending queue: m_nextSendQ
        //    m_nextSendQ will be accessed by server thread to push packets in
        bool m_flushFlag = false;

    private:
        std::mutex m_nextQLock;

    private:
        std::vector<uint8_t> m_currSendQ;
        std::vector<uint8_t> m_nextSendQ;

    public:
        // only asio main loop calls the constructor
        Channel(asio::io_service *, uint32_t);

    public:
        // only asio main loop calls the destructor
        ~Channel();

    public:
        uint32_t id() const
        {
            return m_id;
        }

        auto &socket()
        {
            return m_socket;
        }

    public:
        std::string ip() const
        {
            return m_socket.remote_endpoint().address().to_string();
        }

        uint32_t port() const
        {
            return m_socket.remote_endpoint().port();
        }

    public:
        // family of post facilities, called by one server thread
        // Channel will do compression if needed based on message header code

        // post with a r-ref callback, this is the base of all post function
        // current implementation is based on double-queue method
        // one queue (Q1) is used for store new packages in parallel
        // the other (Q2) queue is used to post all packages in ASIO main loop
        //
        // then Q1 needs to be protected from data race
        // but for Q2 since it's only used in ASIO main loop, we don't need to protect it
        //
        // idea from: https://stackoverflow.com/questions/4029448/thread-safety-for-stl-queue
        void post(uint8_t, const void *, size_t);

    private:
        uint8_t *getPostBuf(size_t bufSize)
        {
            m_postBuf.resize(bufSize + 16);
            return m_postBuf.data();
        }

        uint8_t *getReadBuf(size_t bufSize)
        {
            m_readBuf.resize(bufSize + 16);
            return m_readBuf.data();
        }

        uint8_t *getDecodeBuf(size_t bufSize)
        {
            m_decodeBuf.resize(bufSize + 16);
            return m_decodeBuf.data();
        }

    private:
        std::array<std::tuple<const uint8_t *, size_t>, 2> encodePostBuf(uint8_t, const void *, size_t);

    private:
        // functions called by asio main loop only
        // following DoXXXFunc should only be invoked in asio main loop thread
        void doReadPackHeadCode();
        void doReadPackBody(size_t, size_t);

    private:
        void doSendPack();

    private:
        // called by asio main loop only
        // only called in Channel::doReadPackHeadCode()/doReadPackBody()
        bool forwardActorMessage(uint8_t, const uint8_t *, size_t);

    private:
        // called by one server thread
        // only called in Channel::Post(server_message)
        void flushSendQ();

    private:
        // called by asio main loop thread and server thread
        // it atomically set the channel state, which would disable everything
        void shutdown(bool);

    public:
        void launch();

    public:
        void bindPlayer(uint64_t);
};
