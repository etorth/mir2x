#include <cinttypes>
#include "fflerror.hpp"
#include "serdesmsg.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"
#include "actornetdriver.hpp"
#include "serverargparser.hpp"
#include "serverconfigurewindow.hpp"
#include "serverpeer.hpp"
#include "asiof.hpp"

extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;
extern ServerConfigureWindow *g_serverConfigureWindow;
static thread_local bool t_actorNetThreadFlag = false; // use bool since only has 1 net thread

ActorNetDriver::ActorNetDriver()
    : m_context(std::make_unique<asio::io_context>(1))
{
    launch(g_serverArgParser->getPeerPort());
    if(g_serverArgParser->slave){
        asyncConnect(0, g_serverArgParser->masterIP, g_serverArgParser->masterPort.value(), [this]()
        {
            postMaster(ActorMsgBuf(AM_SYS_SLAVEPEERPORT, cerealf::serialize(SDSysSlavePeerPort
            {
                .port = g_serverArgParser->getPeerPort(),
            })));
        });
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

        if(g_serverArgParser->slave){
            asio::co_spawn(*m_context, readPeerIndex(std::move(sock)), [](std::exception_ptr e)
            {
                if(e){
                    std::rethrow_exception(e);
                }
            });
        }
        else{
            if(m_peerSlotList.empty()){
                m_peerSlotList.emplace_back();
            }

            m_peerSlotList.emplace_back(std::make_unique<PeerSlot>());
            auto slotPtr = m_peerSlotList.back().get();

            slotPtr->sendBuf.clear();
            slotPtr->peer = std::make_shared<ServerPeer>
            (
                this,
                std::move(sock),

                m_peerSlotList.size() - 1,

                slotPtr->lock,
                slotPtr->sendBuf
            );

            auto peer = slotPtr->peer.get();
            g_monoServer->addLog(LOGTYPE_INFO, "Server peer %zu has been connected to master server.", m_peerSlotList.size() - 1);

            peer->launch();
            post(m_peerSlotList.size() - 1, 0, ActorMsgBuf(AM_SYS_PEERINDEX, cerealf::serialize(SDSysPeerIndex
            {
                 .index = m_peerSlotList.size() - 1,
            })));
        }
    }
}

asio::awaitable<void> ActorNetDriver::readPeerIndex(asio::ip::tcp::socket sock)
{
    fflassert(isNetThread());
    std::string buf;

    const auto uid     = co_await asiof::readVLInteger<uint64_t>(sock);
    const auto bufSize = co_await asiof::readVLInteger<  size_t>(sock);

    fflassert(uid == 0);
    fflassert(bufSize > 0);

    buf.resize(bufSize);
    co_await asio::async_read(sock, asio::buffer(buf.data(), buf.size()), asio::use_awaitable);

    const auto mpk = cerealf::deserialize<ActorMsgPack>(buf);
    fflassert(mpk.type() == AM_SYS_PEERINDEX, mpkName(mpk.type()));


    const auto sdPI = cerealf::deserialize<SDSysPeerIndex>(buf);
    fflassert(sdPI.index > 0);

    if(sdPI.index >= m_peerSlotList.size()){
        m_peerSlotList.resize(sdPI.index + 1);
    }

    if(!m_peerSlotList[sdPI.index]){
        m_peerSlotList[sdPI.index] = std::make_unique<PeerSlot>();
    }

    m_peerSlotList[sdPI.index]->sendBuf.clear();
    m_peerSlotList[sdPI.index]->peer = std::make_shared<ServerPeer>
    (
        this,

        std::move(sock),
        sdPI.index,

        m_peerSlotList[sdPI.index]->lock,
        m_peerSlotList[sdPI.index]->sendBuf
    );

    m_peerSlotList[sdPI.index]->peer->launch();
}

void ActorNetDriver::post(size_t peerIndex, uint64_t uid, ActorMsgPack mpk)
{
    logProfiler();

    uint8_t buf[16];
    size_t  bufSize = 0;

    std::string encodeBuf;

    bufSize = msgf::encodeLength(buf, sizeof(buf), uid);
    encodeBuf.append(reinterpret_cast<const char *>(buf), bufSize);

    auto msgBuf = cerealf::serialize(mpk);

    bufSize = msgf::encodeLength(buf, sizeof(buf), msgBuf.size());
    encodeBuf.append(reinterpret_cast<const char *>(buf), bufSize);

    encodeBuf.append(std::move(msgBuf));

    auto slotPtr = m_peerSlotList.at(peerIndex).get();
    if(auto peer = slotPtr->peer){
        {
            const std::lock_guard<std::mutex> lockGuard(slotPtr->lock);
            slotPtr->sendBuf.insert(slotPtr->sendBuf.end(), encodeBuf.begin(), encodeBuf.end());
        }
        peer->notify();
    }
}

void ActorNetDriver::postPeer(size_t peerIndex, ActorMsgPack mpk)
{
    post(peerIndex, 0, std::move(mpk));
}

void ActorNetDriver::postMaster(ActorMsgPack mpk)
{
    postPeer(0, std::move(mpk));
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

void ActorNetDriver::onRemoteMessage(size_t fromPeerIndex, uint64_t uid, ActorMsgPack mpk)
{
    if(uid){
        g_actorPool->postLocalMessage(uid, std::move(mpk));
        return;
    }

    switch(mpk.type()){
        case AM_SYS_PEERINDEX:
            {
                const auto sdPI = mpk.deserialize<SDSysPeerIndex>();
                if(m_peerIndex.has_value()){
                    throw fflerror("invalid request to reassign peer %zu to index %zu", m_peerIndex.value(), sdPI.index);
                }

                m_peerIndex = sdPI.index;
                g_monoServer->addLog(LOGTYPE_INFO, "Assign peer index %zu.", m_peerIndex.value());
                return;
            }
        case AM_SYS_SLAVEPEERPORT:
            {
                const auto sdSPP = mpk.deserialize<SDSysSlavePeerPort>();
                m_remotePeerList[fromPeerIndex] = std::make_pair(m_peerSlotList.at(fromPeerIndex)->peer->ip(), sdSPP.port);

                for(size_t i = 1; i < m_peerSlotList.size(); ++i){
                    post(i, 0, ActorMsgBuf(AM_SYS_SLAVEPEERLIST, cerealf::serialize(SDSysSlavePeerList
                    {
                        .list = m_remotePeerList,
                    })));
                }
                return;
            }
        case AM_SYS_SLAVEPEERLIST:
            {
                const auto sdSPL = mpk.deserialize<SDSysSlavePeerList>();
                for(const auto &[peerIndex, addr]: sdSPL.list){
                    if(peerIndex == 0){
                        throw fflerror("found master peer in slave peer list");
                    }
                    else if(const auto p = m_remotePeerList.find(peerIndex); p != m_remotePeerList.end()){
                        if(p->second != addr){
                            throw fflerror("peer %zu address mismatch", peerIndex);
                        }
                    }
                    else if(peerIndex >= m_peerIndex){
                        continue;
                    }
                    else{
                        if(peerIndex >= m_peerSlotList.size()){
                            m_peerSlotList.resize(peerIndex + 1);
                        }

                        if(m_peerSlotList[peerIndex]){
                            if(m_peerSlotList[peerIndex]->peer){
                                // already connected
                            }
                            else{
                                // connecting
                            }
                        }
                        else{
                            // start to connect
                            m_peerSlotList[peerIndex] = std::make_unique<PeerSlot>();
                            asyncConnect(peerIndex, addr.first, addr.second, [peerIndex, this]()
                            {
                                postPeer(peerIndex, ActorMsgBuf(AM_SYS_PEERINDEX, cerealf::serialize(SDSysPeerIndex
                                {
                                    .index = m_peerIndex.value(),
                                })));
                            });
                        }
                    }
                }
                return;
            }
        case AM_SYS_LAUNCH:
            {
                g_actorPool->launchPool();
                postMaster(AM_SYS_LAUNCHED);
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

        if(peerIndex >= m_peerSlotList.size()){
            m_peerSlotList.resize(peerIndex + 1);
        }

        if(!m_peerSlotList[peerIndex]){
            m_peerSlotList[peerIndex] = std::make_unique<PeerSlot>();
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
