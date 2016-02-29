/*
 * =====================================================================================
 *
 *       Filename: sessionio.cpp
 *        Created: 08/14/2015 11:34:33
 *  Last Modified: 02/29/2016 01:39:34
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
#include "sessionio.hpp"
#include <thread>

SessionIO::SessionIO(int nPort, std::function<void(uint8_t *, size_t)> fnOperateHC)
    : m_Port(nPort)
    , m_EndPoint(asio::ip::tcp::v4(), nPort)
    , m_Acceptor(m_IO, m_EndPoint)
    , m_Socket(m_IO)
    , m_Thread(nullptr)
    , m_MaxID(0)
    , m_OperateFunc(fnOperateHC)
{
}

SessionIO::~SessionIO()
{
}

void SessionIO::Launch()
{
    Accept();
    m_Thread = new std::thread([this](){ m_IO.run(); });
}

void SessionIO::Accept()
{
    auto fnAccept = [this](std::error_code stEC){

        if(stEC){
            // error occurs, stop the network
            Stop();

        }else{

            // m_MonoServer->Log(0, "Connection requested from (%s:%d)",
            //         m_Socket.remote_endpoint().address().to_string().c_str(),
            //         m_Socket.remote_endpoint().port());

            int nSessionID = m_MaxID;

            auto pSession = new Session(
                    nSessionID, std::move(m_Socket), this, m_OperateFunc);

            m_MaxID++;

            m_SessionHub[nSessionID] = pSession;

            pSession->Launch();
        }

        Accept();
    };

    m_Acceptor.async_accept(m_Socket, fnAccept);
}

void SessionIO::Kill(int nSessionID)
{
    if(m_SessionHub.find(nSessionID) != m_SessionHub.end()){
        m_SessionHub[nSessionID]->Stop();
        m_SessionHub.erase(nSessionID);
    }
}

void SessionIO::Dispatch(uint8_t nMsgID, const uint8_t *pData, size_t nDataLen)
{
    for(int nSessionID = 0; nSessionID <= m_MaxID; ++nSessionID){
        auto p = m_SessionHub.find(nSessionID);
        if(p != m_SessionHub.end()){
            p->second->Send(nMsgID, pData, nDataLen);
        }
    }
}

void SessionIO::Send(int nSessionID, uint8_t nMsgID, const uint8_t *pData, size_t nDataLen)
{
    auto p = m_SessionHub.find(nSessionID);
    if(p != m_SessionHub.end()){
        p->second->Send(nMsgID, pData, nDataLen);
    }
}
