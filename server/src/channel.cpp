#include "zcompf.hpp"
#include "channel.hpp"
#include "netdriver.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"
#include "actormsgpack.hpp"

extern NetDriver *g_netDriver;
extern MonoServer *g_monoServer;

Channel::Channel(asio::ip::tcp::socket argSocket, uint32_t argChannID, std::mutex &sendLock, std::vector<uint8_t> &sendBuf)
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

asio::awaitable<void> Channel::reader()
{
    asio::error_code ec;
    while(true){
        co_await asio::async_read(m_socket, asio::buffer(m_readSBuf, 1), asio::redirect_error(asio::use_awaitable, ec));
        checkErrcode(ec);

        m_respIDOpt.reset();
        prepareClientMsg(m_readSBuf[0]);

        if(m_clientMsg->hasResp()){
            m_respIDOpt = co_await readVLInteger<uint64_t>();
        }

        const uint8_t *dataPtr = nullptr;
        /* */  size_t  dataLen = 0;

        switch(m_clientMsg->type()){
            case 0:
                {
                    break;
                }
            case 1:
            case 3:
                {
                    std::tie(dataPtr, dataLen) = co_await readPacketBody(m_clientMsg->maskLen(), co_await readVLInteger<size_t>());
                    break;
                }
            case 2:
                {
                    std::tie(dataPtr, dataLen) = co_await readPacketBody(0, m_clientMsg->dataLen());
                    break;
                }
            default:
                {
                    throw fflvalue(m_clientMsg->name());
                }
        }

        forwardActorMessage(m_clientMsg->headCode(), dataPtr, dataLen, m_respIDOpt.value_or(0));
    }
}

asio::awaitable<std::tuple<const uint8_t *, size_t>> Channel::readPacketBody(size_t maskSize, size_t bodySize)
{
    fflassert(m_clientMsg->checkDataSize(maskSize, bodySize));
    switch(m_clientMsg->type()){
        case 1:
            {
                m_readDBuf.resize(maskSize + bodySize + 64 + m_clientMsg->dataLen());
                break;
            }
        case 2:
            {
                m_readDBuf.resize(m_clientMsg->dataLen());
                break;
            }
        case 3:
            {
                m_readDBuf.resize(bodySize);
                break;
            }
        case 0:
        default:
            {
                throw fflvalue(m_clientMsg->name());
            }
    }

    if(const auto totalSize = maskSize + bodySize){
        asio::error_code ec;
        co_await asio::async_read(m_socket, asio::buffer(m_readDBuf.data(), totalSize), asio::redirect_error(asio::use_awaitable, ec));
        checkErrcode(ec);

        if(maskSize){
            const bool useWide = m_clientMsg->useXor64();
            const size_t dataCount = zcompf::xorCountMask(m_readDBuf.data(), maskSize);

            fflassert(dataCount == (useWide ? (bodySize + 7) / 8 : bodySize));
            fflassert(bodySize <= m_clientMsg->dataLen());

            const auto maskDataPtr = m_readDBuf.data();
            const auto compDataPtr = m_readDBuf.data() + maskSize;
            /* */ auto origDataPtr = m_readDBuf.data() + ((maskSize + bodySize + 7) / 8) * 8;

            const auto decodedBytes = useWide ? zcompf::xorDecode64(origDataPtr, m_clientMsg->dataLen(), maskDataPtr, compDataPtr)
                                              : zcompf::xorDecode  (origDataPtr, m_clientMsg->dataLen(), maskDataPtr, compDataPtr);

            fflassert(decodedBytes == bodySize);
            co_return std::make_tuple(origDataPtr, m_clientMsg->dataLen());
        }
    }
    co_return std::make_tuple(bodySize ? m_readDBuf.data() : nullptr, bodySize);
}

asio::awaitable<void> Channel::writer()
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
                throw ChannelError(id(), "%s", ec.message());
            }
        }
        else{
            asio::error_code ec;
            co_await asio::async_write(m_socket, asio::buffer(m_currSendQ.data(), m_currSendQ.size()), asio::redirect_error(asio::use_awaitable, ec));
            checkErrcode(ec);
            m_currSendQ.clear();
        }
    }
}

bool Channel::forwardActorMessage(uint8_t headCode, const uint8_t *dataPtr, size_t dataLen, uint64_t respID)
{
    fflassert(g_netDriver->isNetThread());
    fflassert(ClientMsg(headCode).checkData(dataPtr, dataLen));

    AMRecvPackage amRP;
    std::memset(&amRP, 0, sizeof(amRP));

    amRP.channID = id();
    buildActorDataPackage(&(amRP.package), headCode, dataPtr, dataLen, respID);
    return m_dispatcher.forward(m_playerUID ? m_playerUID : uidf::getServiceCoreUID(), {AM_RECVPACKAGE, amRP});
}

void Channel::close()
{
    fflassert(g_netDriver->isNetThread());
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
        g_monoServer->addLog(LOGTYPE_WARNING, "Close channel %zu: %s", ec.message());
    }
}

void Channel::launch()
{
    fflassert(g_netDriver->isNetThread());
    asio::co_spawn(m_socket.get_executor(), [self = shared_from_this()]{ return self->reader(); }, +[](std::exception_ptr e){ std::rethrow_exception(e); });
    asio::co_spawn(m_socket.get_executor(), [self = shared_from_this()]{ return self->writer(); }, +[](std::exception_ptr e){ std::rethrow_exception(e); });
}

void Channel::bindPlayer(uint64_t uid)
{
    fflassert(g_netDriver->isNetThread());
    fflassert(uidf::isPlayer(uid), uidf::getUIDString(uid));
    fflassert(!m_playerUID, uidf::getUIDString(m_playerUID), uidf::getUIDString(uid));
    m_playerUID = uid;
}
