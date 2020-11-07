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
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "actorpool.hpp"
#include "netdriver.hpp"
#include "monoserver.hpp"

extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;

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
        throw fflerror("invalid port provided: %llu", to_llu(nPort));
    }

    m_port = nPort;

    try{
        m_IO       = new asio::io_service();
        m_endPoint = new asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_port);
        m_acceptor = new asio::ip::tcp::acceptor(*m_IO, *m_endPoint);
        m_socket   = new asio::ip::tcp::socket(*m_IO);
    }
    catch(...){
        delete m_socket;
        delete m_acceptor;
        delete m_endPoint;
        delete m_IO;

        throw fflerror("initialization of ASIO failed");
    }
    return true;
}

bool NetDriver::Launch(uint32_t nPort, uint64_t nUID)
{
    if(!CheckPort(nPort)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Using invalid port: %llu", to_llu(nPort));
        return false;
    }

    if(g_actorPool->checkInvalid(nUID)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Launch with invaid UID: %llu", to_llu(nUID));
        return false;
    }

    m_serviceCoreUID = nUID;

    if(m_thread.joinable()){
        m_thread.join();
    }

    if(!InitASIO(nPort)){
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
            throw fflerror("get network error when accepting: %s", stEC.message().c_str());
        }

        if(m_channIDQ.Empty()){
            g_monoServer->addLog(LOGTYPE_INFO, "No valid slot for new connection request");

            // currently no valid slot
            // but should wait for new accepting request
            AcceptNewConnection();
            return;
        }

        auto nChannID = m_channIDQ.Head();
        m_channIDQ.PopHead();

        if(!CheckChannID(nChannID)){
            throw fflerror("Get invalid channel ID from reserved queue");
        }

        auto szIP  = m_socket->remote_endpoint().address().to_string();
        auto nPort = m_socket->remote_endpoint().port();

        if(!ChannBuild(nChannID, std::move(*m_socket))){
            g_monoServer->addLog(LOGTYPE_WARNING, "Creating channel for endpoint (%s:%d) failed", szIP.c_str(), nPort);

            // build channel for the allocated id failed
            // recycle the id and post the accept for new request

            m_channIDQ.PushBack(nChannID);
            AcceptNewConnection();
            return;
        }

        auto pChann = m_channelList[nChannID];
        g_monoServer->addLog(LOGTYPE_INFO, "Channel %d established for endpoint (%s:%d)", pChann->ID(), pChann->IP(), pChann->Port());

        // directly lanuch the channel here
        // won't forward the new connection to the service core

        pChann->Launch(m_serviceCoreUID);
        AcceptNewConnection();
    };

    m_acceptor->async_accept(*m_socket, fnAccept);
}
