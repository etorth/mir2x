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

#include "netdriver.hpp"
#include "sysconst.hpp"
#include "monoserver.hpp"

NetDriver::NetDriver()
    : Dispatcher()
    , m_Port(0)
    , m_IO(nullptr)
    , m_EndPoint(nullptr)
    , m_Acceptor(nullptr)
    , m_Socket(nullptr)
    , m_Thread()
    , m_SCAddress(Theron::Address::Null())
    , m_ChannIDQ()
{}

NetDriver::~NetDriver()
{
    m_IO->stop();
    if(m_Thread.joinable()){
        m_Thread.join();
    }

    delete m_Socket;
    delete m_Acceptor;
    delete m_EndPoint;
    delete m_IO;
}

bool NetDriver::CheckPort(uint32_t nPort)
{
    if(nPort <= 1024){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Don't use reserved port: %d", (int)(nPort));
        return false;
    }

    if(m_Port > 1024){
        // TODO here we add other well-unknown occupied port check
    }

    return true;
}

bool NetDriver::InitASIO(uint32_t nPort)
{
    // 1. set server listen port

    if(!CheckPort(nPort)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid port provided");
        g_MonoServer->Restart();
    }

    m_Port = nPort;

    try{
        m_IO       = new asio::io_service();
        m_EndPoint = new asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_Port);
        m_Acceptor = new asio::ip::tcp::acceptor(*m_IO, *m_EndPoint);
        m_Socket   = new asio::ip::tcp::socket(*m_IO);
    }catch(...){
        delete m_Socket;
        delete m_Acceptor;
        delete m_EndPoint;
        delete m_IO;

        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Initialization of ASIO failed");
        g_MonoServer->Restart();
    }

    return true;
}

int NetDriver::Launch(uint32_t nPort, uint64_t nUID)
{
    if(CheckPort(nPort)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Using invalid port: %" PRIu32, nPort);
        return false;
    }

    extern ActorPool *g_ActorPool;
    if(g_ActorPool->CheckInvalid(nUID)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Launch with invaid UID: " PRIu64, nUID);
        return false;
    }

    m_SCUID = nUID;

    if(m_Thread.joinable()){
        m_Thread.join();
    }

    if(!InitASIO(nPort)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "InitASIO failed in NetDriver");
        return false;
    }

    m_ChannIDQ.Clear();
    for(int nIndex = 1; nIndex <= SYS_MAXPLAYERNUM; ++nIndex){
        m_ChannIDQ.PushBack(nIndex);
    }

    AcceptNewConnection();
    m_Thread = std::thread([this]()
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
            // assume g_MonoServer is ready for log
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Get network error when accepting: %s", stEC.message().c_str());
            g_MonoServer->Restart();

            // IO will stop after this
            // won't feed AcceptNewConnection() to event loop again
            return;
        }

        if(m_ChannIDQ.Empty()){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_INFO, "No valid slot for new connection request");

            // currently no valid slot
            // but should wait for new accepting request
            AcceptNewConnection();
            return;
        }

        auto nChannID = m_ChannIDQ.Head();
        m_ChannIDQ.PopHead();

        if(!CheckChannID(nChannID)){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Get invalid channel ID from reserved queue");
            g_MonoServer->Restart();
            return;
        }

        auto szIP  = m_Socket->remote_endpoint().address().to_string();
        auto nPort = m_Socket->remote_endpoint().port();

        if(!ChannBuild(nChannID, std::move(*m_Socket))){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Creating channel for endpoint (%s:%d) failed", szIP.c_str(), nPort);

            // build channel for the allocated id failed
            // recycle the id and post the accept for new request

            m_ChannIDQ.PushBack(nChannID);
            AcceptNewConnection();
            return;
        }

        auto pChann = m_ChannelList[nChannID];

        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_INFO, "Channel %d established for endpoint (%s:%d)", pChann->ID(), pChann->IP(), pChann->Port());

        // directly lanuch the channel here
        // won't forward the new connection to the service core

        pChann->Launch(m_SCAddress);
        AcceptNewConnection();
    };

    m_Acceptor->async_accept(*m_Socket, fnAccept);
}
