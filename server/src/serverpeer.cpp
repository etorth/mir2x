#include <asio.hpp>
#include "asiof.hpp"
#include "server.hpp"
#include "serverpeer.hpp"
#include "actormsgpack.hpp"
#include "actornetdriver.hpp"

extern Server *g_server;

ServerPeer::ServerPeer(ActorNetDriver *argDriver, asio::ip::tcp::socket argSocket, size_t argID, std::mutex &sendLock, std::vector<char> &sendBuf)
    : m_driver(argDriver)
    , m_socket(std::move(argSocket))
    , m_id(argID)
    , m_timer(m_socket.get_executor(), std::chrono::steady_clock::time_point::max())

    // pass sendBuf and sendLock refs to channel
    // sendBuf and sendLock can outlive channel for thread-safe implementation
    , m_nextQLock(sendLock)
    , m_nextSendQ(sendBuf)
{}

asio::awaitable<void> ServerPeer::reader()
{
    std::string buf;
    while(true){
        const auto uid     = co_await asiof::readVLInteger<uint64_t>(m_socket);
        const auto bufSize = co_await asiof::readVLInteger<  size_t>(m_socket);

        fflassert(bufSize > 0);

        buf.resize(bufSize);
        co_await asio::async_read(m_socket, asio::buffer(buf.data(), buf.size()), asio::use_awaitable);

        m_driver->onRemoteMessage(m_id, uid, cerealf::deserialize<ActorMsgPack>(buf));
    }
}

asio::awaitable<void> ServerPeer::writer()
{
    while(m_socket.is_open()){
        {
            const std::lock_guard<std::mutex> lockGuard(m_nextQLock);
            std::swap(m_currSendQ, m_nextSendQ);
        }

        if(m_currSendQ.empty()){
            asio::error_code ec;
            co_await m_timer.asyncWait(asio::redirect_error(asio::use_awaitable, ec));
            if(ec && (ec != asio::error::operation_aborted)){
                throw std::system_error(ec);
            }
        }
        else{
            co_await asio::async_write(m_socket, asio::buffer(m_currSendQ.data(), m_currSendQ.size()), asio::use_awaitable);
            m_currSendQ.clear();
        }
    }
}

void ServerPeer::close()
{
    // For portable behaviour with respect to graceful closure of a connected socket, call shutdown() before closing the socket.
    // asio-1.30.2/doc/asio/reference/basic_stream_socket/close/overload2.html

    std::error_code ec;
    m_socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);

    m_socket.close(ec);
    if(ec){
        g_server->addLog(LOGTYPE_WARNING, "Close peer %zu: %s", id(), ec.message().c_str());
    }
}

void ServerPeer::launch()
{
    fflassert(ActorNetDriver::isNetThread());
    const auto handler = +[](std::exception_ptr e)
    {
        if(e){
            std::rethrow_exception(e);
        }
    };

    asio::co_spawn(m_socket.get_executor(), [self = shared_from_this()]{ return self->reader(); }, handler);
    asio::co_spawn(m_socket.get_executor(), [self = shared_from_this()]{ return self->writer(); }, handler);
}
