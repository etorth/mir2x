#include "zcompf.hpp"
#include "channel.hpp"
#include "netdriver.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"
#include "actormsgpack.hpp"

extern ActorPool *g_actorPool;
extern NetDriver *g_netDriver;
extern MonoServer *g_monoServer;

Channel::Channel(asio::io_service *ioPtr, uint32_t argChannID)
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
{}

Channel::~Channel()
{
    // access raw pointer here
    // don't use shared_from_this() in constructor or destructor

    try{
        close();
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
    switch(m_state.load()){
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
    switch(m_state.load()){
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
                            fflassert(bitCount == to_d(bodySize));
                            fflassert(bodySize <= cmsg.dataLen());

                            decodeMemPtr = channPtr->getDecodeBuf(cmsg.dataLen());
                            const auto decodeSize = zcompf::xorDecode(decodeMemPtr, cmsg.dataLen(), readMemPtr, readMemPtr + maskSize);
                            fflassert(decodeSize == to_d(bodySize));
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

    switch(m_state.load()){
        case CS_RUNNING:
            {
                // will send the first chann pack
                // channel state could switch to STOPPED during sending

                // when we are here
                // we should already have m_flushFlag set as true
                fflassert(m_flushFlag);

                // check m_currSendQ and if it's empty we swap with the pending queue
                // then for server threads calling Post() we only dealing with m_nextSendQ

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
    // flushSendQ() is called by server threads only
    // we post handler fnFlushSendQ to prevent from direct access to m_currSendQ
    m_socket.get_io_service().post([channPtr = shared_from_this()]()
    {
        // m_currSendQ assessing should always be in the asio main loop
        // the Channel::Post() should only access m_nextSendQ

        // then we don't have to put lock to protect m_currSendQ
        // but we need lock for m_nextSendQ, in child threads, in asio main loop

        // but we need to make sure there is only one procedure in asio main loop accessing m_currSendQ
        // because packages in m_currSendQ are divided into two parts: HC / Data
        // one package only get erased after Data is sent
        // then multiple procesdure in asio main loop may send HC / Data more than one time

        // use shared_ptr<Channel>() instead of raw this
        // then outside of asio main loop we use shared_ptr::reset()

        if(!channPtr->m_flushFlag){
            //  mark as current some one is accessing it
            //  we don't even need to make m_flushFlag atomic since it's in one thread
            channPtr->m_flushFlag = true;
            channPtr->doSendPack();
        }
    });
}

std::array<std::tuple<const uint8_t *, size_t>, 2> Channel::encodePostBuf(uint8_t headCode, const void *buf, size_t bufSize)
{
    const ServerMsg smsg(headCode);
    fflassert(smsg.checkData(buf, bufSize));

    switch(smsg.type()){
        case 0:
            {
                return
                {
                    std::make_tuple(nullptr, 0),
                    std::make_tuple(nullptr, 0),
                };
            }
        case 1:
            {
                // not empty, fixed size, comperssed
                // length encoding:
                // [0 - 254]          : length in 0 ~ 254
                // [    255][0 ~ 255] : length as 0 ~ 255 + 255

                // use 1 or 2 bytes
                // variant data length, support range in [0, 255 + 255]

                /* */ auto compBuf = getPostBuf(smsg.maskLen() + bufSize + 64);
                const auto compCnt = zcompf::xorEncode(compBuf + 2, (const uint8_t *)(buf), bufSize);

                fflassert(compCnt >= 0);
                if(compCnt <= 254){
                    compBuf[1] = to_u8(compCnt);
                    return
                    {
                        std::make_tuple(compBuf + 1, 1 + smsg.maskLen() + to_uz(compCnt)),
                        std::make_tuple(nullptr, 0),
                    };
                }
                else if(compCnt <= (255 + 255)){
                    compBuf[0] = 255;
                    compBuf[1] = to_u8(compCnt - 255);
                    return
                    {
                        std::make_tuple(compBuf, 2 + smsg.maskLen() + to_uz(compCnt)),
                        std::make_tuple(nullptr, 0),
                    };
                }
                else{
                    throw fflerror("message length after compression is too long: %d", compCnt);
                }
            }
        case 2:
            {
                // not empty, fixed size, not compressed
                return
                {
                    std::make_tuple((const uint8_t *)(buf), bufSize),
                    std::make_tuple(nullptr, 0),

                };
            }
        case 3:
            {
                // not empty, not fixed size, not compressed
                auto sizeBuf = getPostBuf(4);
                const uint32_t bufSizeU32 = to_u32(bufSize);

                std::memcpy(sizeBuf, &bufSizeU32, 4);
                return
                {
                    std::make_tuple(sizeBuf, 4),
                    std::make_tuple((const uint8_t *)(buf), bufSize),
                };
            }
        default:
            {
                throw bad_reach();
            }
    }
}

void Channel::post(uint8_t headCode, const void *buf, size_t bufLen)
{
    // post current message to m_nextSendQ
    // this function is called by server thread only
    const auto encodedBufList = encodePostBuf(headCode, buf, bufLen);
    {
        std::lock_guard<std::mutex> lockGuard(m_nextQLock);
        m_nextSendQ.push_back(headCode);

        for(const auto &[encodedBuf, encodedBufSize]: encodedBufList){
            if(encodedBuf){
                m_nextSendQ.insert(m_nextSendQ.end(), encodedBuf, encodedBuf + encodedBufSize);
            }
        }
    }

    // when asio thread finish send all pending data it doesn't poll new packs
    // so every time when there is new packs, need to trigger the send, otherwise data stay in the buffer
    flushSendQ();
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
    switch(m_state.exchange(CS_STOPPED)){
        case CS_RUNNING: // actor initiatively close the channel
        case CS_STOPPED: // asio main loop caught exception and close the channel
            {
                AMBadChannel amBC;
                std::memset(&amBC, 0, sizeof(amBC));

                // can forward to servicecore or player
                // servicecore won't keep pointer *this* then we need to report it
                amBC.channID = id();

                m_dispatcher.forward(m_playerUID ? m_playerUID : uidf::getServiceCoreUID(), {AM_BADCHANNEL, amBC});
                m_playerUID = 0;

                // if we call m_socket.shutdown() here
                // we need to use try-catch since if connection has already been broken, it throws exception

                // try{
                //     m_socket.shutdown(asio::ip::tcp::socket::shutdown_both);
                // }
                // catch(...){
                //     ...
                // }

                m_socket.close();
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
    const auto lastState = m_state.exchange(CS_RUNNING);

    fflassert(lastState == CS_NONE);
    doReadPackHeadCode();
}

void Channel::bindPlayer(uint64_t uid)
{
    // force messages forward to the new actor
    // use post rather than directly assignement
    // since asio main loop thread will access m_playerUID
    // this function is called only by actor thread

    // potential bug:
    // internal actor address won't get update immediately after this call

    fflassert(uidf::isPlayer(uid));
    fflassert(g_actorPool->checkUIDValid(uid));

    m_socket.get_io_service().post([uid, this]()
    {
        fflassert(!m_playerUID);
        m_playerUID = uid;
    });
}
