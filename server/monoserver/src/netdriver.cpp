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
    , m_ValidQ()
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

int NetDriver::Launch(uint32_t nPort, const Theron::Address &rstSCAddr)
{
    if(!CheckPort(nPort) || rstSCAddr == Theron::Address::Null()){
        return 1;
    }

    m_SCAddress = rstSCAddr;

    if(m_Thread.joinable()){
        m_Thread.join();
    }

    if(!InitASIO(nPort)){
        return 2;
    }

    // put one accept handler inside the event loop
    // but the asio main loop is not driven by m_Thread yet here
    AcceptNewConnection();
    m_Thread = std::thread([this](){ m_IO->run(); });

    return 0;
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

        if(auto pChann = ChannBuild(std::move(*m_Socket))){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_INFO, "Connection requested from (%s:%d)", pChann->GetIP().c_str(), pChann->GetPort());

            // directly lanuch the channel here
            // won't forward the new connection to the service core
            pCann->Launch(m_SCAddress);
        }else{

            // establish connction error
            // we silently drop the failure here
        }

        AcceptNewConnection();
    };

    m_Acceptor->async_accept(*m_Socket, fnAccept);
}
