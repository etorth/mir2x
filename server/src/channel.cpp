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
    : m_socket([ioPtr     ]() -> asio::io_service & { fflassert(ioPtr)     ; return *ioPtr    ; }())
    , m_id    ([argChannID]() -> uint32_t           { fflassert(argChannID); return argChannID; }())

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

void Channel::afterReadPacketHeadCode()
{
    switch(m_clientMsg->type()){
        case 0:
            {
                forwardActorMessage(m_clientMsg->headCode(), nullptr, 0, m_respIDOpt.value_or(0));
                doReadPacketHeadCode();
                return;
            }
        case 1:
        case 3:
            {
                doReadPacketBodySize();
                return;
            }
        case 2:
            {
                doReadPacketBody(0, m_clientMsg->dataLen());
                return;
            }
        default:
            {
                throw fflvalue(m_clientMsg->name());
            }
    }
}

void Channel::doReadPacketHeadCode()
{
    switch(m_state){
        case CS_RUNNING:
            {
                asio::async_read(m_socket, asio::buffer(m_readSBuf, 1), [channPtr = shared_from_this(), this](std::error_code ec, size_t)
                {
                    checkErrcode(ec);

                    m_respIDOpt.reset();
                    prepareClientMsg(m_readSBuf[0]);

                    if(m_clientMsg->hasResp()){
                        doReadPacketResp();
                    }
                    else{
                        afterReadPacketHeadCode();
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
                throw fflvalue(m_state);
            }
    }
}

void Channel::doReadPacketResp()
{
    doReadVLInteger<uint64_t>(0, [channPtr = shared_from_this(), this](uint64_t respID)
    {
        if(!respID){
            throw fflerror("packet has response encoded but no response id provided");
        }

        m_respIDOpt = respID;
        afterReadPacketHeadCode();
    });
}

void Channel::doReadPacketBodySize()
{
    switch(m_state){
        case CS_RUNNING:
            {
                switch(m_clientMsg->type()){
                    case 1:
                    case 3:
                        {
                            doReadVLInteger<size_t>(0, [channPtr = shared_from_this(), this](size_t bufSize)
                            {
                                doReadPacketBody(m_clientMsg->maskLen(), bufSize);
                            });
                            return;
                        }
                    case 0:
                    case 2:
                    default:
                        {
                            throw fflvalue(m_clientMsg->name());
                        }
                }
            }
        case CS_STOPPED:
            {
                return;
            }
        default:
            {
                throw fflvalue(m_state);
            }
    }
}

void Channel::doReadPacketBody(size_t maskSize, size_t bodySize)
{
    switch(m_state){
        case CS_RUNNING:
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
                    asio::async_read(m_socket, asio::buffer(m_readDBuf.data(), totalSize), [maskSize, bodySize, channPtr = shared_from_this(), this](std::error_code ec, size_t)
                    {
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
                        }

                        forwardActorMessage(m_clientMsg->headCode(), m_readDBuf.data() + (maskSize ? ((maskSize + bodySize + 7) / 8 * 8) : 0), maskSize ? m_clientMsg->dataLen() : bodySize, m_respIDOpt.value_or(0));
                        doReadPacketHeadCode();
                    });
                }
                else{
                    forwardActorMessage(m_clientMsg->headCode(), nullptr, 0, m_respIDOpt.value_or(0));
                    doReadPacketHeadCode();
                }
            }
        case CS_STOPPED:
            {
                return;
            }
        default:
            {
                throw fflvalue(m_state);
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
                asio::async_write(m_socket, asio::buffer(m_currSendQ.data(), m_currSendQ.size()), [sentCount = m_currSendQ.size(), channPtr = shared_from_this(), this](std::error_code ec, size_t)
                {
                    // validate the m_currSendQ size
                    // only asio event loop accesses m_currSendQ and its size should not change during sending
                    checkErrcode(ec);
                    fflassert(sentCount == m_currSendQ.size());

                    m_currSendQ.clear();
                    doSendPack();
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
                throw fflvalue(m_state);
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
                throw fflvalue(m_state);
            }
    }
}

void Channel::launch()
{
    fflassert(g_netDriver->isNetThread());
    fflassert(m_state == CS_NONE, m_state);

    m_state = CS_RUNNING,
    doReadPacketHeadCode();
}

void Channel::bindPlayer(uint64_t uid)
{
    fflassert(g_netDriver->isNetThread());
    fflassert(uidf::isPlayer(uid), uidf::getUIDString(uid));

    fflassert(!m_playerUID, uidf::getUIDString(m_playerUID));
    m_playerUID = uid;
}
