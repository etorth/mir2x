#pragma once
#include <mutex>
#include <concepts>
#include <memory>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <asio.hpp>
#include "strf.hpp"
#include "clientmsg.hpp"
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
            return m_what.empty() ? "unknown channel error" : m_what.c_str();
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
        friend class NetDriver;

    private:
        asio::ip::tcp::socket m_socket;

    private:
        const uint32_t m_id;

    private:
        asio::steady_timer m_timer;

    private:
        Dispatcher m_dispatcher;

    private:
        // for read channel packets
        // only asio main loop thread can access it
        uint8_t m_readSBuf[64];
        std::optional<uint64_t> m_respIDOpt;

    private:
        ClientMsg m_clientMsgBuf;
        const ClientMsg *m_clientMsg = nullptr;

    private:
        std::vector<uint8_t> m_readDBuf;

    private:
        uint64_t m_playerUID = 0;

    private:
        std::mutex &m_nextQLock; // ref to NetDriver::m_channList[channID]::lock

    private:
        std::vector<uint8_t>  m_currSendQ;
        std::vector<uint8_t> &m_nextSendQ; // ref to NetDriver::m_channList[channID]::sendBuf

    public:
        Channel(asio::ip::tcp::socket, uint32_t, std::mutex &, std::vector<uint8_t> &);

    public:
        ~Channel();

    private:
        uint32_t id() const
        {
            return m_id;
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
        void notify()
        {
            m_timer.cancel();
        }

    private:
        void checkErrcode(const asio::error_code &ec) const
        {
            if(ec){
                throw ChannelError(id(), "%s", ec.message());
            }
        }

    private:
        asio::awaitable<void> reader();
        asio::awaitable<void> writer();

    private:
        bool forwardActorMessage(uint8_t, const uint8_t *, size_t, uint64_t);

    private:
        void close();

    private:
        void launch();

    public:
        void bindPlayer(uint64_t);

    private:
        void prepareClientMsg(uint8_t headCode)
        {
            m_clientMsgBuf.~ClientMsg();
            m_clientMsg = new (&m_clientMsgBuf) ClientMsg(headCode);
        }

        template<std::unsigned_integral T> asio::awaitable<T> readVLInteger()
        {
            size_t offset = 0;
            for(size_t offset = 0; offset < (sizeof(T) * 8 + 6) / 7; ++offset){
                co_await asio::async_read(m_socket, asio::buffer(m_readSBuf + offset, 1), asio::deferred);
                if(!(m_readSBuf[offset] & 0x80)){
                    co_return msgf::decodeLength<T>(m_readSBuf, offset + 1);
                }
            }
            throw fflerror("variant packet size uses more than %zu bytes", offset);
        }

    private:
        asio::awaitable<std::tuple<const uint8_t *, size_t>> readPacketBody(size_t, size_t);

    private:
        static void forwardException(std::exception_ptr);
};
