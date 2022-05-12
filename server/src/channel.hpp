#pragma once
#include <asio.hpp>
#include <mutex>
#include <memory>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include "strf.hpp"
#include "dispatcher.hpp"

class ChannelError: public std::exception
{
    private:
        const uint32_t m_channID;

    private:
        std::string m_what;

    public:
        ChannelError(uint32_t channID)
            : m_channID(channID)
        {}

        ChannelError(uint32_t channID, const char *format, ...)
            : m_channID(channID)
        {
            str_format(format, m_what);
        }

    public:
        const char *what() const noexcept override
        {
            return m_what.empty() ? "ChannelError: unknown error" : m_what.c_str();
        }

    public:
        uint32_t channID() const
        {
            return m_channID;
        }
};

class NetDriver;
class Channel final: public std::enable_shared_from_this<Channel>
{
    private:
        enum ChannStateType: int
        {
            CS_NONE     = 0,
            CS_RUNNING  = 1,
            CS_STOPPED  = 2, // stopped by exception
            CS_RELEASED = 3, // socket has been closed, AM_BADCHANNEL has been sent
        };

    private:
        friend class NetDriver;

    private:
        asio::ip::tcp::socket m_socket;

    private:
        const uint32_t m_id;

    private:
        int m_state = CS_NONE;

    private:
        Dispatcher m_dispatcher;

    private:
        // for read channel packets
        // only asio thread accesses these variables
        uint8_t  m_readHeadCode = 0;
        uint8_t  m_readLen[4]   = {0, 0, 0, 0};
        uint32_t m_bodyLen      = 0;

    private:
        std::vector<uint8_t> m_readBuf;
        std::vector<uint8_t> m_decodeBuf;

    private:
        uint64_t m_playerUID = 0;

    private:
        // m_flushFlag indicates there is procedure accessing m_currSendQ in asio thread
        // it prevents more than one procedure from accessing m_currSendQ
        bool m_flushFlag = false;

    private:
        std::mutex &m_nextQLock; // ref to NetDriver::m_channList[channID]::lock

    private:
        std::vector<uint8_t>  m_currSendQ;
        std::vector<uint8_t> &m_nextSendQ; // ref to NetDriver::m_channList[channID]::sendBuf

    public:
        Channel(asio::io_service *, uint32_t, std::mutex &, std::vector<uint8_t> &);

    public:
        ~Channel();

    private:
        uint32_t id() const
        {
            return m_id;
        }

        auto &socket()
        {
            return m_socket;
        }

    private:
        std::string ip() const
        {
            return m_socket.remote_endpoint().address().to_string();
        }

        uint32_t port() const
        {
            return m_socket.remote_endpoint().port();
        }

    private:
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
        void doReadPackHeadCode();
        void doReadPackBody(size_t, size_t);

    private:
        void doSendPack();

    private:
        bool forwardActorMessage(uint8_t, const uint8_t *, size_t);

    private:
        void flushSendQ();

    private:
        void close();

    private:
        void launch();

    public:
        void bindPlayer(uint64_t);
};
