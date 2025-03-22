#include "zcompf.hpp"
#include "channel.hpp"
#include "netdriver.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"
#include "actormsgpack.hpp"

extern MonoServer *g_monoServer;

ServerPeer::ServerPeer(asio::ip::tcp::socket argSocket, uint32_t argChannID, std::mutex &sendLock, std::vector<uint8_t> &sendBuf)
    : m_socket(std::move(argSocket))
    , m_id([argChannID]
      {
          fflassert(argChannID);
          return argChannID;
      }())

    , m_timer(m_socket.get_executor(), std::chrono::steady_clock::time_point::max())

    , m_clientMsgBuf(CM_NONE_0)

    // pass sendBuf and sendLock refs to channel
    // sendBuf and sendLock can outlive channel for thread-safe implementation
    , m_nextQLock(sendLock)
    , m_nextSendQ(sendBuf)
{}

asio::awaitable<void> ServerPeer::reader()
{
    while(true){
        uint64_t uid = 0;
        co_await asio::async_read(m_socket, asio::buffer(std::addressof(uid), sizeof(uid)), asio::use_awaitable);

        size_t bufSize = 0;
        co_await asio::async_read(m_socket, asio::buffer(std::addressof(bufSize), sizeof(bufSize)), asio::use_awaitable);

        std::string buf;
        buf.resize(bufSize);
        co_await asio::async_read(m_socket, asio::buffer(buf.data(), buf.size()), asio::use_awaitable);

        g_actorPool->postMessage(uid, cerealf::deserialize<ActorMsgPack>(buf));
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
            co_await m_timer.async_wait(asio::redirect_error(asio::use_awaitable, ec));
            if(ec != asio::error::operation_aborted){
                throw std::system_error(ec);
            }
        }
        else{
            for(const auto &[uid, mpk]; m_currSendQ){
                co_await asio::async_write(m_socket, asio::buffer(std::addressof(uid), sizeof(uid)), asio::use_awaitable);

                const auto buf = cerealf::serialize(mpk);
                const size_t bufSize = buf.size();

                co_await asio::async_write(m_socket, asio::buffer(std::addressof(bufSize), sizeof(bufSize)), asio::use_awaitable);
                co_await asio::async_write(m_socket, asio::buffer(buf.data(), buf.size()), asio::use_awaitable);
            }
            m_currSendQ.clear();
        }
    }
}

void ServerPeer::close()
{
    fflassert(ActorNetDriver::isNetThread());
    AMBadChannel amBC;
    std::memset(&amBC, 0, sizeof(amBC));

    // can forward to servicecore or player
    // servicecore won't keep pointer *this* then we need to report it
    amBC.channID = id();

    if(m_playerUID){
        m_dispatcher.forward(m_playerUID, {AM_BADCHANNEL, amBC});
    }

    m_playerUID = 0;
    m_dispatcher.forward(uidf::getServiceCoreUID(), {AM_BADCHANNEL, amBC});

    // For portable behaviour with respect to graceful closure of a connected socket, call shutdown() before closing the socket.
    // asio-1.30.2/doc/asio/reference/basic_stream_socket/close/overload2.html

    std::error_code ec;
    m_socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);

    m_socket.close(ec);
    if(ec){
        g_monoServer->addLog(LOGTYPE_WARNING, "Close channel %zu: %s", ec.message().c_str());
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
