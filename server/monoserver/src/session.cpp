/*
 * =====================================================================================
 *
 *       Filename: session.cpp
 *        Created: 9/3/2015 3:48:41 AM
 *  Last Modified: 04/13/2017 21:17:40
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
#include "memorypn.hpp"
#include "monoserver.hpp"

Session::Session(uint32_t nSessionID, asio::ip::tcp::socket stSocket)
    : SyncDriver()
    , m_ID(nSessionID)
    , m_Socket(std::move(stSocket))
    , m_IP(m_Socket.remote_endpoint().address().to_string())
    , m_Port(m_Socket.remote_endpoint().port())
    , m_MessageHC(0)
    , m_BodyLen(0)
    , m_BindAddress(Theron::Address::Null())
    , m_Lock()
    , m_SendQBuf0()
    , m_SendQBuf1()
    , m_CurrSendQ(&(m_SendQBuf0))
    , m_NextSendQ(&(m_SendQBuf1))
{}

Session::~Session()
{
    Shutdown();
}

void Session::DoReadHC()
{
    auto fnDoneReadHC = [this](std::error_code stEC, size_t){
        // ok we get error
        if(stEC){ Shutdown(); return; }

        // now we need to check body
        extern MonoServer *g_MonoServer;
        size_t nLen = g_MonoServer->MessageSize(m_MessageHC);

        // ok it's a non-empty message with fixed body
        if(nLen > 0){ DoReadBody(nLen); DoReadHC(); return; }

        // ok it's a empty message
        if(g_MonoServer->MessageFixedSize(m_MessageHC)){
            AMNetPackage stAMNP;
            stAMNP.SessionID = m_ID;
            stAMNP.Type      = m_MessageHC;
            stAMNP.Data      = nullptr;
            stAMNP.DataLen   = 0;

            // no handler for response, just send
            Forward({MPK_NETPACKAGE, stAMNP}, m_BindAddress);
            DoReadHC();
            return;
        }
        
        // it's a unfixed-length net package
        // 1. read length
        // 2. read body (still maybe zero-length of the body)
        auto fnDoneReadLen = [this](std::error_code stEC, size_t){
            if(stEC){ Shutdown(); return; }
            if(m_BodyLen){ DoReadBody(m_BodyLen); DoReadHC(); return; }

            // ooops, you promised this is a unfixed size message
            // but you send a empty message, ok...
            AMNetPackage stAMNP;
            stAMNP.SessionID = m_ID;
            stAMNP.Type      = m_MessageHC;
            stAMNP.Data      = nullptr;
            stAMNP.DataLen   = 0;

            // no handler for response, just send
            Forward({MPK_NETPACKAGE, stAMNP}, m_BindAddress);
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

    extern MemoryPN *g_MemoryPN;
    auto pData = (uint8_t *)g_MemoryPN->Get(nBodyLen);

    // this actually won't happen
    if(!pData){ Shutdown(); return; }

    auto fnDoneReadBody = [this, nBodyLen, pData](std::error_code stEC, size_t){
        if(stEC){ Shutdown(); return; }

        AMNetPackage stAMNP;
        stAMNP.SessionID = m_ID;
        stAMNP.Type      = m_MessageHC;
        stAMNP.Data      = pData;
        stAMNP.DataLen   = nBodyLen;

        Forward({MPK_NETPACKAGE, stAMNP}, m_BindAddress);
    };

    asio::async_read(m_Socket, asio::buffer(pData, nBodyLen), fnDoneReadBody);
}

void Session::DoSendNext()
{
    assert(!m_CurrSendQ->empty());
    if(std::get<3>(m_CurrSendQ->front())){
        std::get<3>(m_CurrSendQ->front())();
    }

    m_CurrSendQ->pop();
    DoSendHC();
}

void Session::DoSendBuf()
{
    assert(!m_CurrSendQ->empty());
    if(std::get<1>(m_CurrSendQ->front()) && (std::get<2>(m_CurrSendQ->front()) > 0)){
        auto fnDoneSend = [this](std::error_code stEC, size_t){
            if(stEC){ Shutdown(); }else{ DoSendNext(); }
        };
        asio::async_write(m_Socket, asio::buffer(std::get<1>(m_CurrSendQ->front()), std::get<2>(m_CurrSendQ->front())), fnDoneSend);
    }else{
        DoSendNext();
    }
}

void Session::DoSendHC()
{
    if(m_CurrSendQ->empty()){
        std::lock_guard<std::mutex> stLockGuard(m_Lock);
        if(m_NextSendQ->empty()){ return; }
        else{
            std::swap(m_CurrSendQ, m_NextSendQ);
        }
    }

    auto fnDoSendBuf = [this](std::error_code stEC, size_t){
        if(stEC){ Shutdown(); } else{ DoSendBuf(); }
    };
    assert(!m_CurrSendQ->empty());
    asio::async_write(m_Socket, asio::buffer(&(std::get<0>(m_CurrSendQ->front())), 1), fnDoSendBuf);
}
