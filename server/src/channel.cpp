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

Channel::~Channel()
{
    try{
        close();
        {
            const std::lock_guard<std::mutex> lockGuard(m_nextQLock);
            std::vector<uint8_t>().swap(m_nextSendQ);
        }
    }
    catch(const std::exception &e){
        g_monoServer->addLog(LOGTYPE_WARNING, "Failed to release channel %d: %s", to_d(id()), e.what());
    }
    catch(...){
        g_monoServer->addLog(LOGTYPE_WARNING, "Failed to release channel %d: unknown error", to_d(id()));
    }
}

asio::awaitable<void> Channel::reader()
{
    while(true){
        co_await asio::async_read(m_socket, asio::buffer(m_readSBuf, 1), asio::use_awaitable);

        m_respIDOpt.reset();
        prepareClientMsg(m_readSBuf[0]);

        if(m_clientMsg->hasResp()){
            m_respIDOpt = co_await readVLInteger<uint64_t>();
        }

        const uint8_t *dataPtr = nullptr;
        size_t   dataLen = 0;

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
        co_await asio::async_read(m_socket, asio::buffer(m_readDBuf.data(), totalSize), asio::use_awaitable);
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
            co_return std::make_tuple(origDataPtr, bodySize);
        }
    }
    co_return std::make_tuple(bodySize ? m_readDBuf.data() : nullptr, bodySize);
}

asio::awaitable<void> Channel::writer()
{
    while(true){
        {
            const std::lock_guard<std::mutex> lockGuard(m_nextQLock);
            std::swap(m_currSendQ, m_nextSendQ);
        }

        if(m_currSendQ.empty()){
            asio::error_code ec;
            co_await m_timer.async_wait(redirect_error(asio::use_awaitable, ec));
        }
        else{
            const auto sentBytes = co_await asio::async_write(m_socket, asio::buffer(m_currSendQ.data(), m_currSendQ.size()), asio::deferred);
            fflassert(sentBytes == m_currSendQ.size());
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

    // if we call m_socket.shutdown() here
    // we need to use try-catch since if connection has already been broken, it throws exception

    // try{
    //     m_socket.shutdown(asio::ip::tcp::socket::shutdown_both);
    // }
    // catch(...){
    //     ...
    // }

    m_socket.close();
}

void Channel::launch()
{
    fflassert(g_netDriver->isNetThread());
    asio::co_spawn(m_socket.get_executor(), [self = shared_from_this()]{ return self->reader(); }, asio::detached);
    asio::co_spawn(m_socket.get_executor(), [self = shared_from_this()]{ return self->writer(); }, asio::detached);
}

void Channel::bindPlayer(uint64_t uid)
{
    fflassert(g_netDriver->isNetThread());
    fflassert(uidf::isPlayer(uid), uidf::getUIDString(uid));

    fflassert(!m_playerUID, uidf::getUIDString(m_playerUID));
    m_playerUID = uid;
}
