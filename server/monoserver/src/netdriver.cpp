/*
 * =====================================================================================
 *
 *       Filename: netdriver.cpp
 *        Created: 06/25/2017 12:05:00
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include <cinttypes>
#include "sysconst.hpp"
#include "actorpool.hpp"
#include "netdriver.hpp"
#include "monoserver.hpp"

NetDriver::NetDriver()
    : Dispatcher()
    , m_port(0)
    , m_IO(nullptr)
    , m_endPoint(nullptr)
    , m_acceptor(nullptr)
    , m_socket(nullptr)
    , m_thread()
    , m_serviceCoreUID(0)
    , m_channIDQ()
{}

NetDriver::~NetDriver()
{
    m_IO->stop();
    if(m_thread.joinable()){
        m_thread.join();
    }

    delete m_socket;
    delete m_acceptor;
    delete m_endPoint;
    delete m_IO;
}

bool NetDriver::CheckPort(uint32_t nPort)
{
    if(nPort <= 1024){
        extern MonoServer *g_monoServer;
        g_monoServer->addLog(LOGTYPE_WARNING, "Don't use reserved port: %d", (int)(nPort));
        return false;
    }

    if(m_port > 1024){
        // TODO here we add other well-unknown occupied port check
    }

    return true;
}

bool NetDriver::InitASIO(uint32_t nPort)
{
    // 1. set server listen port

    if(!CheckPort(nPort)){
        extern MonoServer *g_monoServer;
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid port provided");
        g_monoServer->Restart();
    }

    m_port = nPort;

    try{
        m_IO       = new asio::io_service();
        m_endPoint = new asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_port);
        m_acceptor = new asio::ip::tcp::acceptor(*m_IO, *m_endPoint);
        m_socket   = new asio::ip::tcp::socket(*m_IO);
    }catch(...){
        delete m_socket;
        delete m_acceptor;
        delete m_endPoint;
        delete m_IO;

        extern MonoServer *g_monoServer;
        g_monoServer->addLog(LOGTYPE_WARNING, "Initialization of ASIO failed");
        g_monoServer->Restart();
    }

    return true;
}

bool NetDriver::Launch(uint32_t nPort, uint64_t nUID)
{
    if(!CheckPort(nPort)){
        extern MonoServer *g_monoServer;
        g_monoServer->addLog(LOGTYPE_WARNING, "Using invalid port: %" PRIu32, nPort);
        return false;
    }

    extern ActorPool *g_actorPool;
    if(g_actorPool->CheckInvalid(nUID)){
        extern MonoServer *g_monoServer;
        g_monoServer->addLog(LOGTYPE_WARNING, "Launch with invaid UID: " PRIu64, nUID);
        return false;
    }

    m_serviceCoreUID = nUID;

    if(m_thread.joinable()){
        m_thread.join();
    }

    if(!InitASIO(nPort)){
        extern MonoServer *g_monoServer;
        g_monoServer->addLog(LOGTYPE_WARNING, "InitASIO failed in NetDriver");
        return false;
    }

    m_channIDQ.Clear();
    for(int nIndex = 1; nIndex <= SYS_MAXPLAYERNUM; ++nIndex){
        m_channIDQ.PushBack(nIndex);
    }

    AcceptNewConnection();
    m_thread = std::thread([this]()
    {
        m_IO->run();
    });

    return true;
}

void NetDriver::AcceptNewConnection()
{
    auto fnAccept = [this](std::error_code stEC)
    {
        if(stEC){
            // error occurs, stop the network
            // assume g_monoServer is ready for log
            extern MonoServer *g_monoServer;
            g_monoServer->addLog(LOGTYPE_WARNING, "Get network error when accepting: %s", stEC.message().c_str());
            g_monoServer->Restart();

            // IO will stop after this
            // won't feed AcceptNewConnection() to event loop again
            return;
        }

        if(m_channIDQ.Empty()){
            extern MonoServer *g_monoServer;
            g_monoServer->addLog(LOGTYPE_INFO, "No valid slot for new connection request");

            // currently no valid slot
            // but should wait for new accepting request
            AcceptNewConnection();
            return;
        }

        auto nChannID = m_channIDQ.Head();
        m_channIDQ.PopHead();

        if(!CheckChannID(nChannID)){
            extern MonoServer *g_monoServer;
            g_monoServer->addLog(LOGTYPE_WARNING, "Get invalid channel ID from reserved queue");
            g_monoServer->Restart();
            return;
        }

        auto szIP  = m_socket->remote_endpoint().address().to_string();
        auto nPort = m_socket->remote_endpoint().port();

        if(!ChannBuild(nChannID, std::move(*m_socket))){
            extern MonoServer *g_monoServer;
            g_monoServer->addLog(LOGTYPE_WARNING, "Creating channel for endpoint (%s:%d) failed", szIP.c_str(), nPort);

            // build channel for the allocated id failed
            // recycle the id and post the accept for new request

            m_channIDQ.PushBack(nChannID);
            AcceptNewConnection();
            return;
        }

        auto pChann = m_channelList[nChannID];

        extern MonoServer *g_monoServer;
        g_monoServer->addLog(LOGTYPE_INFO, "Channel %d established for endpoint (%s:%d)", pChann->ID(), pChann->IP(), pChann->Port());

        // directly lanuch the channel here
        // won't forward the new connection to the service core

        pChann->Launch(m_serviceCoreUID);
        AcceptNewConnection();
    };

    m_acceptor->async_accept(*m_socket, fnAccept);
}
