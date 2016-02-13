/*
 * =====================================================================================
 *
 *       Filename: sessionacceptor.cpp
 *        Created: 8/14/2015 11:34:33 PM
 *  Last Modified: 09/03/2015 8:06:32 PM
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
#include "session.hpp"
#include "sessionacceptor.hpp"
#include "mainwindow.hpp"
#include <thread>

SessionAcceptor::SessionAcceptor(int nPort)
    : m_EndPoint(asio::ip::tcp::v4(), nPort)
    , m_Acceptor(m_IO, m_EndPoint)
    , m_Socket(m_IO)
    , m_Count(0)
{}

void SessionAcceptor::SetOnRecv(std::function<void(const Message &, Session &)> fnOnRecv)
{
    m_OnRecvFunc = fnOnRecv;
}

void SessionAcceptor::Start()
{
    DoAccept();
    m_Thread = new std::thread([this](){ m_IO.run(); });
}

void SessionAcceptor::DoAccept()
{
    auto fnAccept = [this](std::error_code stEC){
        if(!stEC){
            // no idea why here we need move()
            // auto pSession = new Session(m_Socket, this, m_Count);
            
			extern MainWindow *g_MainWindow;
            char szInfo[128];
            std::sprintf(szInfo, "Connection requested from (%s:%d)", 
                    m_Socket.remote_endpoint().address().to_string().c_str(),
                    m_Socket.remote_endpoint().port());

			g_MainWindow->AddLog(0, szInfo);
            auto pSession = new Session(std::move(m_Socket), this, m_Count);

            // auto fnOnRecv = [this](const Message & stMessage, Session & stSession){
            //     extern GateServer *g_GateServer;
            //     g_GateServer->OnClientRecv(stMessage, stSession);
            // };

            if(m_OnRecvFunc){
                pSession->SetOnRecv(m_OnRecvFunc);
            }

            pSession->Start();
            m_SessionList.emplace_back(pSession);
            m_Count++;
        }
        DoAccept();
    };
	m_Acceptor.async_accept(m_Socket, fnAccept);
}

void SessionAcceptor::StopSession(int nSessionID)
{
    for(auto pSession = m_SessionList.begin(); pSession != m_SessionList.end(); ++pSession){
        if((*pSession)->ID() == nSessionID){
            m_SessionList.erase(pSession);
            break;
        }
    }
}

void SessionAcceptor::Dispatch(const Message &stMessage)
{
    for(auto &pSession: m_SessionList){
        pSession->Deliver(stMessage);
    }
}

void SessionAcceptor::Deliver(int nSessionID, const Message &stMessage)
{
    for(auto pSession = m_SessionList.begin(); pSession != m_SessionList.end(); ++pSession){
        if((*pSession)->ID() == nSessionID){
            (*pSession)->Deliver(stMessage);
            break;
        }
    }
}
