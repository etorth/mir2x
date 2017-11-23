/*
 * =====================================================================================
 *
 *       Filename: netdriver.cpp
 *        Created: 06/25/2017 12:05:00
 *  Last Modified: 10/04/2017 16:46:57
 *
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
    : SyncDriver()
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
    Shutdown(0);

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

// TODO stop io before we restart it
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
    // 1. check parameter
    if(!CheckPort(nPort) || rstSCAddr == Theron::Address::Null()){ return 1; }

    // 2. assign the target address
    m_SCAddress = rstSCAddr;

    // 3. make sure the internal thread has ended
    if(m_Thread.joinable()){
        m_Thread.join();
    }

    // 4. prepare valid session ID
    m_ValidQ.Clear();
    for(uint32_t nID = 1; nID < (uint32_t)(std::extent<decltype(m_ChannelList)>::value); ++nID){
        m_ValidQ.PushHead(nID);
    }

    // 5. init ASIO
    if(!InitASIO(nPort)){ return 2; }

    // 6. put one accept handler inside the event loop
    //    but the asio main loop is not driven by m_Thread yet here
    Accept();

    // 7. start the internal thread to driven the loop
    m_Thread = std::thread([this](){ m_IO->run(); });

    // 8. all Launch() function will return 0 when succceeds
    return 0;
}

bool NetDriver::Activate(uint32_t nSessionID, const Theron::Address &rstTargetAddress)
{
    if(nSessionID && nSessionID < (uint32_t)(std::extent<decltype(m_ChannelList)>::value)){
        if(rstTargetAddress == m_SCAddress){
            return m_ChannelList[nSessionID].Launch(rstTargetAddress);
        }else{
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Channel %d is not activated by service core");
            g_MonoServer->Restart();
        }
    }
    return false;
}

void NetDriver::Accept()
{
    auto fnAccept = [this](std::error_code stEC)
    {
        extern MonoServer *g_MonoServer;
        if(stEC){
            // error occurs, stop the network
            // assume g_MonoServer is ready for log
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Get network error when accepting: %s", stEC.message().c_str());
            g_MonoServer->Restart();

            // IO will stop after this
            // won't feed Accept() to event loop again
            return;
        }

        auto nReqPort = m_Socket->remote_endpoint().port();
        auto szIPAddr = m_Socket->remote_endpoint().address().to_string();
        g_MonoServer->AddLog(LOGTYPE_INFO, "Connection requested from (%s:%d)", szIPAddr.c_str(), nReqPort);

        if(m_ValidQ.Empty()){
            g_MonoServer->AddLog(LOGTYPE_INFO, "No valid slot for new connection request");

            // currently no valid slot
            // but should wait for new accepting request
            Accept();
            return;
        }

        auto nValidID = m_ValidQ.Head();
        m_ValidQ.PopHead();

        if(!nValidID){
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Get zero from reserved session ID queue");
            g_MonoServer->Restart();
            return;
        }

        // use channel nValidID to host the accept
        // if not use std::move() we'll get ``already open" error
        m_ChannelList[nValidID].ChannBuild(nValidID, std::move(*m_Socket));

        // forward a message by SyncDriver::Forward()
        // inform the serice core that there is a new connection
        AMNewConnection stAMNC;
        stAMNC.SessionID = nValidID;

        if(Forward({MPK_NEWCONNECTION, stAMNC}, m_SCAddress)){
            m_ValidQ.PushHead(nValidID);
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Can't inform servicec core for connection id = %d", (int)(nValidID));
            return;
        }

        // notification sent
        // but service core may failed to receive it
        // print log message in ServiceCore::OperateAM() instead here

        // accept next request
        Accept();
    };

    m_Acceptor->async_accept(*m_Socket, fnAccept);
}
