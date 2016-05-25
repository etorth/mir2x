/*
 * =====================================================================================
 *
 *       Filename: sessionhub.cpp
 *        Created: 08/14/2015 11:34:33
 *  Last Modified: 05/24/2016 17:51:59
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

SessionHub::SessionHub(int nPort, const Theron::Address &rstAddress)
    : SyncDriver()
    , m_Port(nPort)
    , m_EndPoint(asio::ip::tcp::v4(), nPort)
    , m_Acceptor(m_IO, m_EndPoint)
    , m_Socket(m_IO)
    , m_Count(1)
    , m_Thread(nullptr)
    , m_SCAddress(rstAddress)
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
            auto pNewSession = new Session(ValidID(), std::move(m_Socket), this);

            if(!pNewSession->ID()){
                g_MonoServer->AddLog(LOGTYPE_INFO, "No valid connection slot, refused");
                pNewSession->Send(SM_REFUSE, [pNewSession](){ delete pNewSession; });

                // this deletion will happen in the same thread, but out of class Session
                // since it's in asio's main loop, should be OK
                return;
            }

            // previously here we start the session fully
            // now instead we'll send it to the service core the get confirm or refuse
            // TODO & TBD: add more info if needed and send it to ServiceCore, to 
            //             implement more like IP ban, user priority etc. currently
            //             I only put the SessionID here

            // forward and no wait here
            uint32_t nSessionID = pNewSession->ID();
            if(Forward({MPK_NEWCONNECTION, nSessionID}, m_SCAddress)){
                delete pNewSession; return;
            }

            // then we put it in the hub, and when the SC respond we can launch it
            m_SessionV[pNewSession->ID()] = pNewSession;

            // accept next request
            Accept();
        }
    };

    m_Acceptor.async_accept(m_Socket, fnAccept);
}

void SessionHub::Launch()
{
    if(m_SCAddress != Theron::Address::Null()){
        Accept();
        m_Thread = new std::thread([this](){ m_IO.run(); });
    }
}

void SessionHub::Shutdown(uint32_t nSID)
{
    // shutdown all sessions
    if(nSID == 0){
        for(auto &pSession: m_SessionV){
            pSession->Shutdown();
            delete pSession;
            pSession = nullptr;
        }
        return;
    }

    m_SessionV[nSID]->Shutdown();
    delete m_SessionV[nSID];
    m_SessionV[nSID] = nullptr;
}
