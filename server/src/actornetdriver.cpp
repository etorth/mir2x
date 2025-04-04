#include <cinttypes>
#include "fflerror.hpp"
#include "serdesmsg.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"
#include "actornetdriver.hpp"
#include "serverargparser.hpp"
#include "serverconfigurewindow.hpp"
#include "serverpeer.hpp"

extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;
extern ServerConfigureWindow *g_serverConfigureWindow;
static thread_local bool t_actorNetThreadFlag = false; // use bool since only has 1 net thread

ActorNetDriver::ActorNetDriver()
    : m_context(std::make_unique<asio::io_context>(1))
{
    launch(g_serverArgParser->peerPort.value_or([]()
    {
        if(g_serverArgParser->slave){
            return g_serverArgParser->masterPort.value();
        }
        else{
            return g_serverArgParser->clientPort.value() + 1;
        }
    }()));

    if(g_serverArgParser->slave){
        asyncConnect(0, g_serverArgParser->masterIP, g_serverArgParser->masterPort.value(), nullptr);
    }
}

ActorNetDriver::~ActorNetDriver()
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

bool ActorNetDriver::isNetThread()
{
    return t_actorNetThreadFlag;
}

void ActorNetDriver::launch(asio::ip::port_type port)
{
    fflassert(!isNetThread());
    fflassert(port > 1024, port);

    m_context  = std::make_unique<asio::io_context>(1);
    m_acceptor = std::make_unique<asio::ip::tcp::acceptor>(*m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));

    asio::co_spawn(*m_context, listener(), [](std::exception_ptr e)
    {
        if(e){
            std::rethrow_exception(e);
        }
    });

    m_thread = std::thread([this]()
    {
        t_actorNetThreadFlag = true;
        m_context->run();
    });
}

asio::awaitable<void> ActorNetDriver::listener()
{
    fflassert(isNetThread());

    asio::error_code ec;
    while(true){
        auto sock = co_await m_acceptor->async_accept(asio::redirect_error(asio::use_awaitable, ec));
        if(ec){
            m_acceptor->close();
            throw fflerror("acceptor error: %s", ec.message().c_str());
        }

        m_peerSlotList.emplace_back();
        auto slotPtr = m_peerSlotList.back().get();

        slotPtr->sendBuf.clear();
        slotPtr->peer = std::make_shared<ServerPeer>(this, std::move(sock), m_peerSlotList.size(), slotPtr->lock, slotPtr->sendBuf);

        auto peer = slotPtr->peer.get();
        // g_monoServer->addLog(LOGTYPE_INFO, "Channel %zu has been established for endpoint (%s:%d).", to_d(peer->peerIndex()), to_cstr(peer->ip()), to_d(peer->port()));

        peer->launch();

        SDSysNotifySlave sdSNS
        {
            .slaveID = m_peerSlotList.size(),
        };

        for(const auto &slot: m_peerSlotList){
            sdSNS.peerList.emplace_back(slot->peer->ip(), slot->peer->port());
        }

        post(m_peerSlotList.size(), 0, ActorMsgBuf(AM_SYS_NOTIFYSLAVE, cerealf::serialize(sdSNS)));
    }
}

void ActorNetDriver::post(size_t peerIndex, uint64_t uid, ActorMsgPack mpk)
{
    logProfiler();

    uint8_t buf[16];
    size_t  bufSize = 0;

    m_sendBuf.clear();

    bufSize = msgf::encodeLength(buf, sizeof(buf), uid);
    m_sendBuf.append(reinterpret_cast<const char *>(buf), bufSize);

    auto msgBuf = cerealf::serialize(mpk);

    bufSize = msgf::encodeLength(buf, sizeof(buf), msgBuf.size());
    m_sendBuf.append(reinterpret_cast<const char *>(buf), bufSize);

    m_sendBuf.append(std::move(msgBuf));

    auto slotPtr = m_peerSlotList.at(peerIndex).get();
    if(auto peer = slotPtr->peer){
        {
            const std::lock_guard<std::mutex> lockGuard(slotPtr->lock);
            slotPtr->sendBuf.insert(slotPtr->sendBuf.end(), m_sendBuf.begin(), m_sendBuf.end());
        }
        peer->notify();
    }
}

void ActorNetDriver::postMaster(ActorMsgPack mpk)
{
    post(0, 0, mpk);
}

void ActorNetDriver::close(uint32_t channID)
{
    logProfiler();
    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_peerSlotList.size());
    fflassert(g_actorPool->isActorThread());

    // if actor thread would initialize a shutdown to a channel
    // it should call this function to schedule a shutdown event via m_context->post()

    // after this function call, the channel slot can still be not empty
    // player actor should keep a flag(m_channID.has_value() && m_channID.value() == 1) to indicate it shall not post any new message

    std::atomic_flag closeDone;
    asio::post(*m_context, [channID, &closeDone, this]()
    {
        // TODO shall I add any validation to confirm that only the bind player UID can close the channel?
        //      otherwise a careless call to ActorNetDriver::close() with random channID can crash other player's connection
        doClose(channID);
        closeDone.test_and_set();
        closeDone.notify_all();
    });

    closeDone.wait(false);
    auto slotPtr = m_peerSlotList.at(channID).get();
    {
        const std::lock_guard<std::mutex> lockGuard(slotPtr->lock);
        std::vector<char>().swap(slotPtr->sendBuf);
    }
}

void ActorNetDriver::doRelease()
{
    if(m_context){
        m_context->stop();
    }

    if(m_thread.joinable()){
        m_thread.join();
    }
}

void ActorNetDriver::doClose(uint32_t channID)
{
    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_peerSlotList.size());
    fflassert(isNetThread());

    if(m_peerSlotList[channID]->peer){
        m_peerSlotList[channID]->peer->close();
        m_peerSlotList[channID]->peer->notify();
        m_peerSlotList[channID]->peer.reset();
    }
}

size_t ActorNetDriver::hasPeer() const
{
    size_t count = 0;
    for(const auto &p: m_peerSlotList){
        if(p){
            count++;
        }
    }
    return count;
}

void ActorNetDriver::onRemoteMessage(uint64_t uid, ActorMsgPack mpk)
{
    if(uid){
        g_actorPool->postLocalMessage(uid, std::move(mpk));
        return;
    }

    switch(mpk.type()){
        case AM_SYS_NOTIFYSLAVE:
            {
                const auto sdSNS = mpk.deserialize<SDSysNotifySlave>();
                m_peerIndex = sdSNS.slaveID;

                for(size_t id = 0; const auto &[ip, port]: sdSNS.peerList){
                    asyncConnect(id, ip, port, [ip]
                    {
                        g_monoServer->addLog(LOGTYPE_INFO, "%s", to_cstr(ip));
                    });
                }
                return;
            }
        case AM_SYS_LAUNCH:
            {
                g_actorPool->launchPool();
                return;
            }
        default:
            {
                return;
            }
    }
}

void ActorNetDriver::asyncConnect(size_t peerIndex, const std::string &ip, asio::ip::port_type port, std::function<void()> afterLaunch)
{
    auto masterSock = std::make_shared<asio::ip::tcp::socket>(*m_context);
    asio::async_connect(*masterSock, asio::ip::tcp::resolver(*m_context).resolve(ip, std::to_string(port)), [peerIndex, masterSock, afterLaunch = std::move(afterLaunch), this](std::error_code ec, const asio::ip::tcp::endpoint &)
    {
        if(ec){
            throw fflerror("network error: %s", ec.message().c_str());
        }

        m_peerSlotList[peerIndex]->sendBuf.clear();
        m_peerSlotList[peerIndex]->peer = std::make_shared<ServerPeer>
        (
            this,

            std::move(*masterSock),
            peerIndex,

            m_peerSlotList[peerIndex]->lock,
            m_peerSlotList[peerIndex]->sendBuf
        );

        m_peerSlotList[peerIndex]->peer->launch();
        if(afterLaunch){
            afterLaunch();
        }
    });
}

void ActorNetDriver::closeAcceptor()
{
    m_acceptor->close();
}
