#include "zcompf.hpp"
#include "channel.hpp"
#include "netdriver.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"
#include "actormsgpack.hpp"

extern ActorPool *g_actorPool;
extern NetDriver *g_netDriver;
extern MonoServer *g_monoServer;

Channel::Channel(asio::io_service *ioPtr, uint32_t argChannID, std::mutex &sendLock, std::vector<uint8_t> &sendBuf)
    : m_socket([ioPtr]() -> asio::io_service &
      {
          fflassert(ioPtr);
          return *ioPtr;
      }())
    , m_id([argChannID]()
      {
          fflassert(argChannID);
          return argChannID;
      }())

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
            std::lock_guard<std::mutex> lockGuard(m_nextQLock);
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

// need to setup state by: channPtr->m_state = CS_STOPPED
// otherwise if there are more completions handlers associated to this channel, they won't get acknowledged that this channel shouldn't be used any more
#define _abort_channel_if_errcode(channPtr, ec) \
do{ \
    if(ec){ \
        (channPtr)->m_state = CS_STOPPED; \
        throw ChannError((channPtr)->id(), "network error on channel %d: %s", (channPtr)->id(), (ec).message().c_str()); \
    } \
}while(0)

void Channel::doReadPackHeadCode()
{
    switch(m_state){
        case CS_RUNNING:
            {
                asio::async_read(m_socket, asio::buffer(&m_readHeadCode, 1), [channPtr = shared_from_this()](std::error_code ec, size_t)
                {
                    _abort_channel_if_errcode(channPtr, ec);
                    const ClientMsg cmsg(channPtr->m_readHeadCode);
                    switch(cmsg.type()){
                        case 0:
                            {
                                channPtr->forwardActorMessage(channPtr->m_readHeadCode, nullptr, 0);
                                channPtr->doReadPackHeadCode();
                                return;
                            }
                        case 1:
                            {
                                // not empty, fixed size, compressed
                                asio::async_read(channPtr->m_socket, asio::buffer(channPtr->m_readLen, 1), [channPtr, cmsg](std::error_code ec, size_t)
                                {
                                    _abort_channel_if_errcode(channPtr, ec);
                                    if(channPtr->m_readLen[0] != 255){
                                        fflassert(to_uz(channPtr->m_readLen[0]) <= cmsg.dataLen());
                                        channPtr->doReadPackBody(cmsg.maskLen(), channPtr->m_readLen[0]);
                                    }
                                    else{
                                        // oooops, bytes[0] is 255
                                        // we got a long message and need to read bytes[1]
                                        asio::async_read(channPtr->m_socket, asio::buffer(channPtr->m_readLen + 1, 1), [channPtr, cmsg](std::error_code ec, size_t)
                                        {
                                            _abort_channel_if_errcode(channPtr, ec);
                                            fflassert(channPtr->m_readLen[0] == 255);
                                            const auto compSize = to_uz(channPtr->m_readLen[1]) + 255;

                                            fflassert(compSize <= cmsg.dataLen());
                                            channPtr->doReadPackBody(cmsg.maskLen(), compSize);
                                        });
                                    }
                                });
                                return;
                            }
                        case 2:
                            {
                                // not empty, fixed size, not compressed

                                // it has no overhead, fast
                                // this mode should be used for small messages
                                channPtr->doReadPackBody(0, cmsg.dataLen());
                                return;
                            }
                        case 3:
                            {
                                // not empty, not fixed size, not compressed

                                // directly read four bytes as length
                                // this mode is designed for transfering big chunk
                                // for even bigger chunk we should send more than one time

                                // and if we want to send big chunk of compressed data
                                // we should compress it by other method and send it via this channel
                                asio::async_read(channPtr->m_socket, asio::buffer(channPtr->m_readLen, 4), [channPtr](std::error_code ec, size_t)
                                {
                                    _abort_channel_if_errcode(channPtr, ec);
                                    channPtr->doReadPackBody(0, to_uz(as_u32(channPtr->m_readLen)));
                                });
                                return;
                            }
                        default:
                            {
                                throw bad_reach();
                            }
                    }
                });
                return;
            }
        case CS_STOPPED:
            {
                return;
            }
        default:
            {
                throw bad_reach();
            }
    }
}

void Channel::doReadPackBody(size_t maskSize, size_t bodySize)
{
    switch(m_state){
        case CS_RUNNING:
            {
                const ClientMsg cmsg(m_readHeadCode);
                fflassert(cmsg.checkDataSize(maskSize, bodySize));

                if(const auto totalSize = maskSize + bodySize){
                    auto readMemPtr = getReadBuf(totalSize);
                    asio::async_read(m_socket, asio::buffer(readMemPtr, totalSize), [channPtr = shared_from_this(), maskSize, bodySize, readMemPtr, cmsg](std::error_code ec, size_t)
                    {
                        _abort_channel_if_errcode(channPtr, ec);
                        uint8_t *decodeMemPtr = nullptr;
                        if(maskSize){
                            const auto bitCount = zcompf::countMask(readMemPtr, maskSize);
                            fflassert(bitCount == bodySize);
                            fflassert(bodySize <= cmsg.dataLen());

                            decodeMemPtr = channPtr->getDecodeBuf(cmsg.dataLen());
                            const auto decodeSize = zcompf::xorDecode(decodeMemPtr, cmsg.dataLen(), readMemPtr, readMemPtr + maskSize);
                            fflassert(decodeSize == bodySize);
                        }

                        channPtr->forwardActorMessage(channPtr->m_readHeadCode, decodeMemPtr ? decodeMemPtr : readMemPtr, maskSize ? cmsg.dataLen() : bodySize);
                        channPtr->doReadPackHeadCode();
                    });
                }
                else{
                    forwardActorMessage(m_readHeadCode, nullptr, 0);
                    doReadPackHeadCode();
                }
                return;
            }
        case CS_STOPPED:
            {
                return;
            }
        default:
            {
                throw bad_reach();
            }
    }
}

void Channel::doSendPack()
{
    // 1. only called in asio main loop thread
    // 2. only called in RUNNING / STOPPED state

    switch(m_state){
        case CS_RUNNING:
            {
                // will send the first chann pack
                // channel state could switch to STOPPED during sending

                // when we are here
                // we should already have m_flushFlag set as true
                fflassert(m_flushFlag);

                // check m_currSendQ and if it's empty we swap with the pending queue
                // then for server threads calling post() we only dealing with m_nextSendQ

                if(m_currSendQ.empty()){
                    std::lock_guard<std::mutex> lockGuard(m_nextQLock);
                    if(m_nextSendQ.empty()){
                        // neither queue contains pending packages
                        // mark m_flushFlag as no one accessing m_currSendQ and return
                        m_flushFlag = false;
                        return;
                    }
                    else{
                        // else we still need to access m_currSendQ
                        // keep m_flushFlag to pervent other thread to call DoSendHC()
                        std::swap(m_currSendQ, m_nextSendQ);
                    }
                }

                fflassert(!m_currSendQ.empty());
                asio::async_write(m_socket, asio::buffer(m_currSendQ.data(), m_currSendQ.size()), [channPtr = shared_from_this(), sentCount = m_currSendQ.size()](std::error_code ec, size_t)
                {
                    // validate the m_currSendQ size
                    // only asio event loop accesses m_currSendQ and its size should not change during sending
                    _abort_channel_if_errcode(channPtr, ec);
                    fflassert(sentCount == channPtr->m_currSendQ.size());
                    channPtr->m_currSendQ.clear();
                    channPtr->doSendPack();
                });
                return;
            }
        case CS_STOPPED:
            {
                // asio can't cancel a handler after post
                // so we have to check it here and exit directly if channel stopped
                // nothing is important now, we just return and won't take care of m_flushFlag
                return;
            }
        default:
            {
                throw bad_reach();
            }
    }
}

void Channel::flushSendQ()
{
    // m_currSendQ assessing should always be in the asio main loop
    // the Channel::post() should only access m_nextSendQ

    // then we don't have to put lock to protect m_currSendQ
    // but we need lock for m_nextSendQ, in child threads, in asio main loop

    // but we need to make sure there is only one procedure in asio main loop accessing m_currSendQ
    // because packages in m_currSendQ are divided into two parts: HC / Data
    // one package only get erased after Data is sent
    // then multiple procesdure in asio main loop may send HC / Data more than one time

    fflassert(g_netDriver->isNetThread());
    if(!m_flushFlag){
        //  mark as current some one is accessing it
        //  we don't even need to make m_flushFlag atomic since it's in one thread
        m_flushFlag = true;
        doSendPack();
    }
}

bool Channel::forwardActorMessage(uint8_t headCode, const uint8_t *dataPtr, size_t dataLen)
{
    fflassert(g_netDriver->isNetThread());
    fflassert(ClientMsg(headCode).checkData(dataPtr, dataLen));

    AMRecvPackage amRP;
    std::memset(&amRP, 0, sizeof(amRP));

    amRP.channID = id();
    buildActorDataPackage(&(amRP.package), headCode, dataPtr, dataLen);
    return m_dispatcher.forward(m_playerUID ? m_playerUID : uidf::getServiceCoreUID(), {AM_RECVPACKAGE, amRP});
}

void Channel::close()
{
    fflassert(g_netDriver->isNetThread());
    switch(m_state){
        case CS_RUNNING: // actor initiatively close the channel
        case CS_STOPPED: // asio main loop caught exception and close the channel
            {
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
                m_state = CS_RELEASED;
                return;
            }
        default:
            {
                throw bad_reach();
            }
    }
}

void Channel::launch()
{
    fflassert(g_netDriver->isNetThread());
    fflassert(m_state == CS_NONE);

    m_state = CS_RUNNING,
    doReadPackHeadCode();
}

void Channel::bindPlayer(uint64_t uid)
{
    fflassert(g_netDriver->isNetThread());
    fflassert(uidf::isPlayer(uid));

    fflassert(!m_playerUID);
    m_playerUID = uid;
}
