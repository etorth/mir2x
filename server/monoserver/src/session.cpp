/*
 * =====================================================================================
 *
 *       Filename: session.cpp
 *        Created: 9/3/2015 3:48:41 AM
 *  Last Modified: 05/26/2016 00:48:39
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

#include <cassert>
#include "session.hpp"
#include "sessionhub.hpp"

Session::Session(uint32_t nSessionID, asio::ip::tcp::socket stSocket, SessionHub *pSessionHub)
    : m_ID(nSessionID)
    , m_Socket(std::move(stSocket))
    , m_SessionHub(pSessionHub)
    , m_IP(m_Socket.remote_endpoint().address().to_string())
    , m_Port(m_Socket.remote_endpoint().port())
    , m_MessageHC(0)
    , m_BodyLen(0)
{}

Session::~Session()
{
    Stop();
}

void Session::DoReadHC()
{
    auto fnDoneReadHC = [this](std::error_code stEC, size_t){
        // ok we get error
        if(stEC){ Stop(); return; }

        // now we need to check body
        extern MonoServer *g_MonoServer;
        size_t nLen = g_MonoServer->MessageSize(m_MessageHC);

        // ok it's a non-empty message with fixed body
        if(nLen > 0){ DoReadBody(nLen); DoReadHC(); return; }

        // ok it's a empty message
        if(g_MonoServer->MessageFixedSize(m_MessageHC)){
            AMNetPackage stAMNP;
            stAMNP.SessionID = m_ID;
            stAMNP.HC        = m_MessageHC;
            stAMNP.Data      = nullptr;
            stAMNP.DataLen   = 0;

            // no handler for response, just send
            Forward(m_TargetAddress, {MPK_NETPACKAGE, stAMNP});
            DoReadHC();
            return;
        }
        
        // it's a unfixed-length net package
        // 1. read length
        // 2. read body (still maybe zero-length of the body)
        auto fnDoneReadLen = [this](std::error_code stEC, size_t){
            if(stEC){ Stop(); return; }
            if(m_BodyLen){ DoReadBody(m_BodyLen); DoReadHC(); return; }

            // ooops, you promised this is a unfixed size message
            // but you send a empty message, ok...
            AMNetPackage stAMNP;
            stAMNP.SessionID = m_ID;
            stAMNP.HC        = m_MessageHC;
            stAMNP.Data      = nullptr;
            stAMNP.DataLen   = 0;

            // no handler for response, just send
            Forward(m_TargetAddress, {MPK_NETPACKAGE, stAMNP});
            DoReadHC();
        };
        asio::async_read(m_Socket, asio::buffer(&m_BodyLen, sizeof(m_BodyLen)), fnDoneReadLen);
    };

    asio::async_read(m_Socket, asio::buffer(&m_MessageHC, 1), fnDoneReadHC);
}

void Session::DoReadBody(size_t nBodyLen)
{
    if(!nBodyLen){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "message body length should be positive");
        g_MonoServer->Restart();
    }

    extern MemoryChunkPN *g_MemoryChunkPN;
    auto pData = (uint8_t *)g_MemoryChunkPN->Get(nBodyLen);

    // this actually won't happen
    if(!pData){ Stop(); return; }

    auto fnDoneReadBody = [this, nBodyLen, pData](std::error_code stEC, size_t){
        if(stEC){ Stop(); return; }

        AMNetPackage stAMNP;
        stAMNP.SessionID = m_ID;
        stAMNP.HC        = m_MessageHC;
        stAMNP.Data      = pData;
        stAMNP.DataLen   = nBodyLen;

        Forward(m_TargetAddress, {MPK_NETPACKAGE, stAMNP});
    };

    asio::async_read(m_Socket, asio::buffer(pData, nBodyLen), fnDoneReadBody);
}

// we assume there always be at least one SendTaskDesc
// since it's a callback after a send task done
void Session::DoSendNext()
{
    // 1. invoke the callback if needed
    if(std::get<3>(m_SendQ.front())){ std::get<3>(m_SendQ.front())(); }

    // 2. remove the front
    m_SendQ.pop();

    // 3. do send if we have more work
    if(!m_SendQ.empty()){ DoSendHC(); }
}

void Session::DoSendBuf()
{
    if(m_SendQ.empty()){ return; }

    if(std::get<1>(m_SendQ.front()) && (std::get<2>(m_SendQ.front()) > 0)){
        auto fnDoSendValidBuf = [this](std::error_code stEC, size_t){
            if(stEC){ Stop(); }else{ DoSendNext(); }
        };

        asio::async_write(m_Socket,
                asio::buffer(std::get<1>(m_SendQ.front()), std::get<2>(m_SendQ.front())),
                fnDoSendValidBuf);
    }else{
        DoSendNext();
    }
}

void Session::DoSendHC()
{
    if(m_SendQ.empty()){ return; }

    auto fnDoSendBuf = [this](std::error_code stEC, size_t){
        if(stEC){ Stop(); }else{ DoSendBuf(); }
    };

    asio::async_write(m_Socket, asio::buffer(&(std::get<0>(m_SendQ.front())), 1), fnDoSendBuf);
}
