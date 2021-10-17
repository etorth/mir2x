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
        release();
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
    fflassert(m_port == 0);

    m_port = port;
    m_io = new asio::io_service();
    m_endPoint = new asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_port);
    m_acceptor = new asio::ip::tcp::acceptor(*m_io, *m_endPoint);

    fflassert(!m_channIDQ.empty());
    acceptNewConnection();

    m_thread = std::thread([this]()
    {
        t_netThreadFlag = true;
        m_io->run();
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
