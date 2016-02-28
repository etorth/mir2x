/*
 * =====================================================================================
 *
 *       Filename: sessionmanager.cpp
 *        Created: 08/14/2015 11:34:33
 *  Last Modified: 02/28/2016 00:07:47
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

SessionManager::SessionManager(int nPort)
    : m_EndPoint(asio::ip::tcp::v4(), nPort)
    , m_Acceptor(m_IO, m_EndPoint)
    , m_Socket(m_IO)
    , m_Count(0)
{}

void SessionManager::SetOnRecv(std::function<void(const Message &, Session &)> fnOnRecv)
{
    m_OnRecvFunc = fnOnRecv;
}

void SessionManager::Start(std::function<void()> fnOperationOnHC)
{
    DoAccept(fnOperationOnHC);
    m_Thread = new std::thread([this](){ m_IO.run(); });
}

void SessionManager::DoAccept(std::function<void()> fnOperationOnHC)
{
    auto fnAccept = [this, fnOperationOnHC](std::error_code stEC){

        if(stEC){
            // error occurs, stop the network
            Stop();

        }else{

            m_MonoServer->Log(0, "Connection requested from (%s:%d)",
                    m_Socket.remote_endpoint().address().to_string().c_str(),
                    m_Socket.remote_endpoint().port());

            int nSessionID = AllocateID();

            auto pSession = new Session(
                    nSessionID, std::move(m_Socket), this, fnOperationOnHC);

            m_SessionHub[nSessionID] = pSession;

            pSession->Start();

            m_SessionList.emplace_back(pSession);
        }

        DoAccept(fnOperationOnHC);
    };

    m_Acceptor.async_accept(m_Socket, fnAccept);
}

void SessionManager::StopSession(int nSessionID)
{
    if(m_SessionHub.find(nSessionID) != m_SessionHub.end()){
        m_SessionHub[nSessionID]->Stop();
        m_SessionHub.erase(nSessionID);
    }
}

void SessionManager::Dispatch(uint8_t nMsgID, const uint8_t *pData, int nDataLen)
{
    for(auto &pSession: m_SessionList){
        pSession->Deliver(nMsgID, pData, nDataLen);
    }
}

void SessionManager::Deliver(int nSessionID, uint8_t nMsgID, const uint8_t *pData)
{
    for(auto pSession = m_SessionList.begin(); pSession != m_SessionList.end(); ++pSession){
        if((*pSession)->ID() == nSessionID){
            (*pSession)->Deliver(stMessage);
            break;
        }
    }
}

int SessionManager::AllocateID()
{
    static int nNextID = 0;

    while(m_SessionHub.find(nNextID) != m_SessionHub.end()){
        nNextID++;
    }

    return nNextID;
}

int SessionManager::SessionCount()
{
    return m_SessionHub.size();
}
