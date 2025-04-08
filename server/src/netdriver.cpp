#include <cinttypes>
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "actorpool.hpp"
#include "channel.hpp"
#include "netdriver.hpp"
#include "monoserver.hpp"
#include "serverconfigurewindow.hpp"

extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;
extern ServerConfigureWindow *g_serverConfigureWindow;
static thread_local bool t_netThreadFlag = false; // use bool since only has 1 net thread

NetDriver::NetDriver()
    : m_channelSlotList(g_serverConfigureWindow->getConfig().maxPlayerCount + 1)
{
    // allocate channal IDs
    // reserve the first slot with zero channel ID
    for(size_t channID = 1; channID < m_channelSlotList.size(); ++channID){
        m_channelIDList.push_back(channID);
    }
}

NetDriver::~NetDriver()
{
    try{
        doRelease();
    }
    catch(const std::exception &e){
        g_monoServer->addLog(LOGTYPE_WARNING, "Failed when release net driver: %s", e.what());
    }
    catch(...){
        g_monoServer->addLog(LOGTYPE_WARNING, "Failed when release net driver: unknown error");
    }
}

bool NetDriver::isNetThread()
{
    return t_netThreadFlag;
}

void NetDriver::launch(uint32_t port)
{
    fflassert(port > 1024, port);
    fflassert(g_actorPool->checkUIDValid(uidf::getServiceCoreUID(0)));

    fflassert(!m_port);
    fflassert(!isNetThread());
    fflassert(!m_channelIDList.empty());

    m_port = static_cast<asio::ip::port_type>(port);
    m_context = std::make_unique<asio::io_context>(1);

    asio::co_spawn(*m_context, listener(), [](std::exception_ptr e)
    {
        if(e){
            std::rethrow_exception(e);
        }
    });

    m_thread = std::thread([this]()
    {
        t_netThreadFlag = true;

        // net driver shouldn't crash the main loop when exception get thrown in completion handlers
        // see: http://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/reference/io_service.html#boost_asio.reference.io_service.effect_of_exceptions_thrown_from_handlers
        while(true){
            try{
                m_context->run();
                break; // run() exited normally
            }
            catch(const ChannelError &e){
                doClose(e.channID());
                g_monoServer->addLog(LOGTYPE_INFO, "Channel %d has been disconnected.", to_d(e.channID()));
            }
            // only catch channError
            // let it crash when caught any other exceptions
        }
    });
}

asio::awaitable<void> NetDriver::listener()
{
    fflassert(isNetThread());

    asio::error_code ec;
    asio::ip::tcp::acceptor acceptor(*m_context, {asio::ip::tcp::v4(), m_port});

    while(true){
        auto sock = co_await acceptor.async_accept(asio::redirect_error(asio::use_awaitable, ec));
        if(ec){
            acceptor.close();
            throw fflerror("acceptor error: %s", ec.message().c_str());
        }

        if(m_channelIDList.empty()){
            // TODO notify client that server is busy
        }
        else{
            const auto channID = m_channelIDList.front();
            m_channelIDList.pop_front();

            fflassert(to_uz(channID) > 0, channID);
            fflassert(to_uz(channID) < m_channelSlotList.size(), channID, m_channelSlotList.size());


            auto slotPtr = m_channelSlotList.data() + channID;

            slotPtr->sendBuf.clear();
            slotPtr->channPtr = std::make_shared<Channel>(std::move(sock), channID, slotPtr->lock, slotPtr->sendBuf);

            auto channPtr = m_channelSlotList[channID].channPtr.get();
            g_monoServer->addLog(LOGTYPE_INFO, "Channel %d has been established for endpoint (%s:%d).", to_d(channPtr->id()), to_cstr(channPtr->ip()), to_d(channPtr->port()));

            channPtr->launch();
        }
    }
}

std::array<std::tuple<const uint8_t *, size_t>, 2> NetDriver::encodePostBuf(uint8_t headCode, const void *buf, size_t bufSize, uint64_t respID, std::vector<uint8_t> &encodeBuf)
{
    const ServerMsg smsg(headCode);
    fflassert(smsg.checkData(buf, bufSize));

    encodeBuf.clear();

    if(respID){
        uint8_t respBuf[32];
        const size_t respEncSize = msgf::encodeLength(respBuf, 32, respID);

        encodeBuf.push_back(headCode | 0x80);
        encodeBuf.insert(encodeBuf.end(), respBuf, respBuf + respEncSize);
    }
    else{
        encodeBuf.push_back(headCode);
    }

    switch(smsg.type()){
        case 0:
            {
                return
                {
                    std::make_tuple(encodeBuf.data(), encodeBuf.size()),
                    std::make_tuple(nullptr, 0),
                };
            }
        case 1:
            {
                const size_t sizeStartOff = encodeBuf.size();
                encodeBuf.resize(sizeStartOff + smsg.maskLen() + smsg.dataLen() + 64);

                /* */ auto sizeBuf = encodeBuf.data() + sizeStartOff;
                /* */ auto compBuf = encodeBuf.data() + sizeStartOff + 32;
                const auto compCnt = smsg.useXor64 () ? zcompf::xorEncode64(compBuf, (const uint8_t *)(buf), bufSize)
                                                      : zcompf::xorEncode  (compBuf, (const uint8_t *)(buf), bufSize);

                const size_t sizeEncLength = msgf::encodeLength(sizeBuf, 32, compCnt);
                return
                {
                    std::make_tuple(encodeBuf.data(), sizeStartOff + sizeEncLength),
                    std::make_tuple(compBuf, smsg.maskLen() + compCnt),
                };
            }
        case 2:
            {
                return
                {
                    std::make_tuple(encodeBuf.data(), encodeBuf.size()),
                    std::make_tuple((const uint8_t *)(buf), bufSize),
                };
            }
        case 3:
            {
                const auto sizeStartOff = encodeBuf.size();
                encodeBuf.resize(encodeBuf.size() + 32);

                const auto encodedLenBytes = msgf::encodeLength(encodeBuf.data() + sizeStartOff, 32, bufSize);
                encodeBuf.resize(sizeStartOff + encodedLenBytes);

                return
                {
                    std::make_tuple(encodeBuf.data(), encodeBuf.size()),
                    std::make_tuple((const uint8_t *)(buf), bufSize),
                };
            }
        default:
            {
                throw fflvalue(smsg.name());
            }
    }
}

void NetDriver::post(uint32_t channID, uint8_t headCode, const void *buf, size_t bufSize, uint64_t respID)
{
    logProfiler();
    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_channelSlotList.size());
    fflassert(ServerMsg(headCode).checkData(buf, bufSize));

    if(headCode >= 0x80){
        throw fflerror("invalid head code %02d", to_d(headCode));
    }

    // post current message to sendBuf, which links to Channel::m_nextSendQ
    // this function is called by server thread only
    //
    // current implementation is based on double-queue method
    // one queue (Q1) is used for store new packages in parallel
    // the other (Q2) queue is used to post all packages in ASIO main loop
    //
    // then Q1 needs to be protected from data race
    // but for Q2 since it's only used in ASIO main loop, we don't need to protect it
    //
    // idea from: https://stackoverflow.com/questions/4029448/thread-safety-for-stl-queue
    auto slotPtr = m_channelSlotList.data() + channID;
    if(auto channPtr = slotPtr->channPtr){
        const auto encodedBufList = encodePostBuf(headCode, buf, bufSize, respID, slotPtr->encodeBuf);
        {
            const std::lock_guard<std::mutex> lockGuard(slotPtr->lock);
            for(const auto &[encodedBuf, encodedBufSize]: encodedBufList){
                if(encodedBuf){
                    slotPtr->sendBuf.insert(slotPtr->sendBuf.end(), encodedBuf, encodedBuf + encodedBufSize);
                }
            }
        }
        channPtr->notify();
    }
}

void NetDriver::bindPlayer(uint32_t channID, uint64_t uid)
{
    logProfiler();
    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_channelSlotList.size());
    fflassert(uidf::isPlayer(uid));

    std::atomic_flag waitDone;
    asio::post(*m_context, [uid, channID, &waitDone, this]()
    {
        // use post rather than directly assignement since asio thread accesses m_playerUID
        // potential bug is it's done by post so actor uid is not updated immediately after this call

        if(auto channPtr = m_channelSlotList[channID].channPtr.get()){
            channPtr->bindPlayer(uid);
        }

        waitDone.test_and_set();
        waitDone.notify_all();
    });
    waitDone.wait(false);
}

void NetDriver::close(uint32_t channID)
{
    logProfiler();
    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_channelSlotList.size());
    fflassert(g_actorPool->isActorThread());

    // if actor thread would initialize a shutdown to a channel
    // it should call this function to schedule a shutdown event via m_context->post()

    // after this function call, the channel slot can still be not empty
    // player actor should keep a flag(m_channID.has_value() && m_channID.value() == 1) to indicate it shall not post any new message

    std::atomic_flag closeDone;
    asio::post(*m_context, [channID, &closeDone, this]()
    {
        // TODO shall I add any validation to confirm that only the bind player UID can close the channel?
        //      otherwise a careless call to NetDriver::close() with random channID can crash other player's connection
        doClose(channID);
        closeDone.test_and_set();
        closeDone.notify_all();
    });

    closeDone.wait(false);
    auto slotPtr = m_channelSlotList.data() + channID;
    {
        const std::lock_guard<std::mutex> lockGuard(slotPtr->lock);
        std::vector<uint8_t>().swap(slotPtr->sendBuf);
    }
}

void NetDriver::doRelease()
{
    if(m_context){
        m_context->stop();
    }

    if(m_thread.joinable()){
        m_thread.join();
    }
}

void NetDriver::doClose(uint32_t channID)
{
    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_channelSlotList.size());
    fflassert(isNetThread());

    if(m_channelSlotList[channID].channPtr){
        m_channelSlotList[channID].channPtr->close();
        m_channelSlotList[channID].channPtr->notify();
        m_channelSlotList[channID].channPtr.reset();
        m_channelIDList.push_back(channID);
    }
}
