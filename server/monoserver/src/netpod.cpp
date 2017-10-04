/*
 * =====================================================================================
 *
 *       Filename: netpod.cpp
 *        Created: 06/25/2017 12:05:00
 *  Last Modified: 10/03/2017 21:46:49
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

#include "netpod.hpp"
#include "sysconst.hpp"
#include "monoserver.hpp"

NetPodN::NetPodN()
    : SyncDriver()
    , m_Port(0)
    , m_IO(nullptr)
    , m_EndPoint(nullptr)
    , m_Acceptor(nullptr)
    , m_Socket(nullptr)
    , m_Thread()
    , m_ValidQ()
    , m_SCAddress(Theron::Address::Null())
{
    for(size_t nSID = 0; nSID < SYS_MAXPLAYERNUM; ++nSID){
        m_SessionV[0][nSID] = nullptr;
        m_SessionV[1][nSID] = nullptr;
    }
}

NetPodN::~NetPodN()
{
    Shutdown(0);
    if(m_Thread.joinable()){
        m_Thread.join();
    }

    delete m_Socket;
    delete m_Acceptor;
    delete m_EndPoint;
    delete m_IO;
}

Session* NetPodN::Validate(uint32_t nSessionID, bool bValid)
{
    if(true
            && nSessionID > 0
            && nSessionID < SYS_MAXPLAYERNUM){
        return m_SessionV[bValid ? 1 : 0][nSessionID];
    }
    return nullptr;
}

bool NetPodN::CheckPort(uint32_t nPort)
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
bool NetPodN::InitASIO(uint32_t nPort)
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

int NetPodN::Launch(uint32_t nPort, const Theron::Address &rstSCAddr)
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
    for(uint32_t nID = 1; nID < SYS_MAXPLAYERNUM; ++nID){
        m_ValidQ.PushHead(nID);
    }

    // 5. init ASIO
    if(!InitASIO(nPort)){ return 2; }

    // 6. put one accept handler inside the event loop
    Accept();

    // 7. start the internal thread
    m_Thread = std::thread([this](){ m_IO->run(); });

    // 8. all Launch() function will return 0 when succceeds
    return 0;
}

int NetPodN::Activate(uint32_t nSessionID, const Theron::Address &rstTargetAddress)
{
    // 1. check argument
    if(false
            || nSessionID == 0
            || nSessionID >= SYS_MAXPLAYERNUM
            || rstTargetAddress == Theron::Address::Null()){
        return 1;
    }

    // 2. get pointer
    if(!m_SessionV[0][nSessionID]){ return 2; }

    // 3. corresponding running slot is empty?
    if(m_SessionV[1][nSessionID]){ return 3; }

    // 4. start it and move it to the running slot
    if(m_SessionV[0][nSessionID]->Launch(rstTargetAddress)){ return 4; }

    // 5. launch ok
    std::swap(m_SessionV[0][nSessionID], m_SessionV[1][nSessionID]);
    m_SessionV[0][nSessionID] = nullptr;

    return 0;
}

void NetPodN::Accept()
{
    auto fnAccept = [this](std::error_code stEC)
    {
        extern MonoServer *g_MonoServer;
        if(stEC){
            // error occurs, stop the network
            // assume g_MonoServer is ready for log
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Get network error when accepting");
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

        if(m_SessionV[0][nValidID]){
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Get in-using session id from reserved session ID queue");
            g_MonoServer->Restart();
            return;
        }

        m_SessionV[0][nValidID] = new Session(nValidID, std::move(*m_Socket));

        // should forward a message by SyncDriver::Forward()
        // inform the serice core that there is a new connection
        AMNewConnection stAMNC;
        stAMNC.SessionID = nValidID;

        if(Forward({MPK_NEWCONNECTION, stAMNC}, m_SCAddress)){
            delete m_SessionV[0][nValidID];
            m_SessionV[0][nValidID] = nullptr;

            m_ValidQ.PushHead(nValidID);
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Can't inform ServiceCore a new connection");
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
