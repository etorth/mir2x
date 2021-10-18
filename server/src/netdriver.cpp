#include <cinttypes>
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "actorpool.hpp"
#include "netdriver.hpp"
#include "monoserver.hpp"

extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;
static thread_local bool t_netThreadFlag = false; // use bool since only has 1 net thread

NetDriver::NetDriver()
    : m_channList(SYS_MAXPLAYERNUM + 1)
{
    // reserve all valid channal IDs
    // don't use the first slot with ID as zero
    for(size_t channID = 1; channID < m_channList.size(); ++channID){
        m_channIDQ.push(channID);
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

bool NetDriver::isNetThread() const
{
    return t_netThreadFlag;
}

void NetDriver::launch(uint32_t port)
{
    fflassert(port > 1024);
    fflassert(g_actorPool->checkUIDValid(uidf::getServiceCoreUID()));

    fflassert(!m_port);
    fflassert(!isNetThread());

    m_port = port;
    m_io = new asio::io_service();
    m_endPoint = new asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_port);
    m_acceptor = new asio::ip::tcp::acceptor(*m_io, *m_endPoint);

    fflassert(!m_channIDQ.empty());
    acceptNewConnection();

    m_thread = std::thread([this]()
    {
        t_netThreadFlag = true;

        // net driver shouldn't crash the main loop when exception get thrown in completion handlers
        // see: http://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/reference/io_service.html#boost_asio.reference.io_service.effect_of_exceptions_thrown_from_handlers
        while(true){
            try{
                m_io->run();
                break; // run() exited normally
            }
            catch(const ChannError &e){
                doClose(e.channID());
                g_monoServer->addLog(LOGTYPE_WARNING, "Channel %d disconnected.", to_d(e.channID()));
            }
            // only catch channError
            // let it crash when caught any other exceptions
        }
    });
}

void NetDriver::acceptNewConnection()
{
    if(m_channIDQ.empty()){
        g_monoServer->addLog(LOGTYPE_INFO, "No valid slot for new connection, request ignored.");
        return;
    }

    const auto channID = m_channIDQ.front();
    m_channIDQ.pop();

    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_channList.size());

    m_channList[channID] = std::make_shared<Channel>(m_io, channID);
    m_acceptor->async_accept(m_channList[channID]->socket(), [channID, this](std::error_code ec)
    {
        if(ec){
            throw fflerror("get network error when accepting new connection: %s", ec.message().c_str());
        }

        auto channPtr = m_channList.at(channID).get();
        g_monoServer->addLog(LOGTYPE_INFO, "Channel %d established for endpoint (%s:%d)", to_d(channPtr->id()), to_cstr(channPtr->ip()), to_d(channPtr->port()));

        channPtr->launch();
        acceptNewConnection();
    });
}

void NetDriver::post(uint32_t channID, uint8_t headCode, const void *buf, size_t bufLen)
{
    logProfiler();
    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_channList.size());
    fflassert(ServerMsg(headCode).checkData(buf, bufLen));
    fflassert(m_channList[channID]);
    m_channList[channID]->post(headCode, buf, bufLen);
}

void NetDriver::bindPlayer(uint32_t channID, uint64_t uid)
{
    logProfiler();
    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_channList.size());
    fflassert(uidf::isPlayer(uid));
    fflassert(m_channList[channID]);
    m_channList[channID]->bindPlayer(uid);
}

void NetDriver::close(uint32_t channID)
{
    logProfiler();
    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_channList.size());
    fflassert(m_channList[channID]);

    // actor thread can access m_channList[channID] directly
    // but it can not release the channel, channel release should only happens in net thread

    // if actor thread would initialize a shutdown to a channel
    // it should call this function to schedule a shutdown event via m_io->post()

    // after this function call, the channel slot can still be not empty
    // player actor should keep a flag(m_channID.has_value() && m_channID.value() == 1) to indicate it shall not post more message

    m_io->post([channID, this]()
    {
        doClose(channID);
    });
}

void NetDriver::doRelease()
{
    if(m_io){
        m_io->stop();
    }

    if(m_thread.joinable()){
        m_thread.join();
    }

    delete m_acceptor;
    delete m_endPoint;
    delete m_io;

    m_acceptor = nullptr;
    m_endPoint = nullptr;
    m_io       = nullptr;
}

void NetDriver::doClose(uint32_t channID)
{
    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_channList.size());
    fflassert(isNetThread());

    if(m_channList[channID]){
        m_channList.at(channID).reset();
        m_channIDQ.push(channID);
    }
}
