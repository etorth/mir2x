#pragma once
#include <mutex>
#include <memory>
#include <vector>
#include <cstdint>
#include <asio.hpp>

class ActorNetDriver;
class ServerPeer final: public std::enable_shared_from_this<ServerPeer>
{
    private:
        friend class ActorNetDriver;

    private:
        ActorNetDriver *m_driver;

    private:
        asio::ip::tcp::socket m_socket;

    private:
        const size_t m_id;

    private:
        asio::steady_timer m_timer;

    private:
        std::mutex &m_nextQLock; // ref to NetDriver::m_channList[channID]::lock

    private:
        std::vector<char>  m_currSendQ;
        std::vector<char> &m_nextSendQ; // ref to NetDriver::m_channList[channID]::sendBuf

    public:
        ServerPeer(ActorNetDriver *, asio::ip::tcp::socket, size_t, std::mutex &, std::vector<char> &);

    public:
        asio::awaitable<void> reader();
        asio::awaitable<void> writer();

    public:
        void close();
        void launch();

    public:
        uint32_t id() const
        {
            return m_id;
        }

    private:
        void notify()
        {
            m_timer.cancel();
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
};
