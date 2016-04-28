/*
 * =====================================================================================
 *
 *       Filename: sessionhub.cpp
 *        Created: 08/14/2015 11:34:33
 *  Last Modified: 04/27/2016 23:14:42
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

#include <thread>

#include "message.hpp"
#include "session.hpp"
#include "sessionhub.hpp"
#include "monoserver.hpp"

SessionHub::SessionHub(int nPort,
        const Theron::Address &rstAddress,
        const std::function<void(uint8_t, Session *)> &fnOperateHC)
    : SyncDriver()
    , m_Port(nPort)
    , m_EndPoint(asio::ip::tcp::v4(), nPort)
    , m_Acceptor(m_IO, m_EndPoint)
    , m_Socket(m_IO)
    , m_MaxID(1)
    , m_Thread(nullptr)
    , m_ServiceCoreAddress(rstAddress)
    , m_OperateFunc(fnOperateHC)
{
}

SessionHub::~SessionHub()
{
    Shutdown();
    m_Thread->join();
    delete m_Thread;
}

void SessionHub::Accept()
{
    auto fnAccept = [this](std::error_code stEC){
        if(stEC){
            // error occurs, stop the network
            // assume g_MonoServer is ready for log
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "network error");
            Shutdown();

            // we won't put Accept() in the event loop again
            // then the IO will stop after this
        }else{
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_INFO, "Connection requested from (%s:%d)",
                    m_Socket.remote_endpoint().address().to_string().c_str(),
                    m_Socket.remote_endpoint().port());

            // 1. create and put into the hub
            //    overwrite existing slot if wrapped back
            auto pNewSession = new Session(m_MaxID++, std::move(m_Socket), this, m_OperateFunc);

            // previously here we start the session fully
            // now instead we'll send it to the service core the get confirm or refuse

            // 2. clear the catcher
            MessagePack stMPK(MPK_NEWCONNECTION, pNewSession, 1);
            MessagePack stRetMPK;
            int nRet = Send(stMPK, m_ServiceCoreAddress, &stRetMPK);

            if(!nRet){
                g_MonoServer->AddLog(LOGTYPE_INFO, "connection from (%s:%d) allowed",
                        m_Socket.remote_endpoint().address().to_string().c_str(),
                        m_Socket.remote_endpoint().port());

                auto pRecord = m_SessionMap.find(pNewSession->ID());
                if(pRecord != m_SessionMap.end()){
                    // ooops, overflow
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "session hub overflows");
                    g_MonoServer->Restart();
                }

                // 1. keep it in the hub
                m_SessionMap[pNewSession->ID()] = pNewSession;

                // 2. echo to the client
                pNewSession->Send(SM_PING);

                // 3. ready login information
                pNewSession->Launch();
            }else{
                // refused or errors
                g_MonoServer->AddLog(LOGTYPE_INFO, "connection from (%s:%d) dropped",
                        m_Socket.remote_endpoint().address().to_string().c_str(),
                        m_Socket.remote_endpoint().port());
                // 1. so we can't use this ID
                m_MaxID--;

                // 2. tell the client you are refused, then delete the session
                pNewSession->Send(SM_REFUSE, [pNewSession](){ delete pNewSession; });
            }

            // accept next request
            Accept();
        }
    };

    m_Acceptor.async_accept(m_Socket, fnAccept);
}

void SessionHub::Launch()
{
    Accept();
    m_Thread = new std::thread([this](){ m_IO.run(); });
}

void SessionHub::Shutdown(uint32_t nSID)
{
    // shutdown all sessions
    if(nSID == 0){
        for(auto &rstRecord: m_SessionMap){
            // check validation of session ID
            if(rstRecord.first == 0){
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_WARNING, "zero id in session map");
            }

            rstRecord.second->Shutdown();
            delete rstRecord.second;
        }
        m_SessionMap.clear();
        return;
    }

    // shutdown specified session
    auto pRecord = m_SessionMap.find(nSID);
    if(pRecord != m_SessionMap.end()){
        pRecord->second->Shutdown();
        delete pRecord->second;
        m_SessionMap.erase(pRecord);
    }
}
