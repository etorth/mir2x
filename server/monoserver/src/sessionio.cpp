/*
 * =====================================================================================
 *
 *       Filename: sessionio.cpp
 *        Created: 08/14/2015 11:34:33
 *  Last Modified: 02/28/2016 22:25:06
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

SessionIO::SessionIO(int nPort)
    : m_EndPoint(asio::ip::tcp::v4(), nPort)
    , m_Acceptor(m_IO, m_EndPoint)
    , m_Socket(m_IO)
    , m_Count(0)
{}

void SessionIO::SetOnRecv(std::function<void(const Message &, Session &)> fnOnRecv)
{
    m_OnRecvFunc = fnOnRecv;
}

void SessionIO::Launch(std::function<void()> fnOperateHC)
{
    Accept(fnOperateHC);
    m_Thread = new std::thread([this](){ m_IO.run(); });
}

void SessionIO::Accept(std::function<void()> fnOperateHC)
{
    auto fnAccept = [this, fnOperateHC](std::error_code stEC){

        if(stEC){
            // error occurs, stop the network
            Stop();

        }else{

            // m_MonoServer->Log(0, "Connection requested from (%s:%d)",
            //         m_Socket.remote_endpoint().address().to_string().c_str(),
            //         m_Socket.remote_endpoint().port());

            int nSessionID = AllocateID();

            auto pSession = new Session(nSessionID, std::move(m_Socket), this, fnOperateHC);

            m_SessionHub[nSessionID] = pSession;

            m_SessionList.emplace_back(pSession);

            pSession->Launch(fnOperateHC);
        }

        Accept(fnOperateHC);
    };

    m_Acceptor.async_accept(m_Socket, fnAccept);
}

void SessionIO::StopSession(int nSessionID)
{
    if(m_SessionHub.find(nSessionID) != m_SessionHub.end()){
        m_SessionHub[nSessionID]->Stop();
        m_SessionHub.erase(nSessionID);
    }
}

void SessionIO::Dispatch(uint8_t nMsgID, const uint8_t *pData, int nDataLen)
{
    for(auto &pSession: m_SessionList){
        pSession->Deliver(nMsgID, pData, nDataLen);
    }
}

void SessionIO::Deliver(int nSessionID, uint8_t nMsgID, const uint8_t *pData)
{
    for(auto pSession = m_SessionList.begin(); pSession != m_SessionList.end(); ++pSession){
        if((*pSession)->ID() == nSessionID){
            (*pSession)->Deliver(stMessage);
            break;
        }
    }
}

int SessionIO::AllocateID()
{
    static int nNextID = 0;

    while(m_SessionHub.find(nNextID) != m_SessionHub.end()){
        nNextID++;
    }

    return nNextID;
}

int SessionIO::SessionCount()
{
    return m_SessionHub.size();
}
