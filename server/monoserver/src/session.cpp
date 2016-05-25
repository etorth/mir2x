/*
 * =====================================================================================
 *
 *       Filename: session.cpp
 *        Created: 9/3/2015 3:48:41 AM
 *  Last Modified: 05/24/2016 15:33:52
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
    auto fnOnReadHC = [this](std::error_code stEC, size_t){
        // ok we get error
        if(stEC){ Stop(); return; }

        // now we need to check body
        extern MonoServer *g_MonoServer;
        size_t nLen = g_MonoServer->MessageSize(m_MessageHC);

        // ok it's a non-empty message with fixed body
        if(nLen > 0){ DoReadBody(nLen); return; }

        // ok it's a empty message
        if(g_MonoServer->MessageFixedSize(m_MessageHC)){
            AMNetPackage stAMNP;
            stAMNP.SessionID = m_ID;
            stAMNP.HC        = m_MessageHC;
            stAMNP.Data      = nullptr;
            stAMNP.DataLen   = 0;

            // no handler for response, just send
            Forward(m_TargetAddress, {MPK_NETPACKAGE, stAMNP});
            return;
        }
        
        // it's a unfixed-length net package
        DoReadBody(m_MessageHC, 0);
    };

    asio::async_read(m_Socket, asio::buffer(&m_MessageHC, 1), fnOnReadHC);
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

void Session::DoReadAfterHC(size_t nMsgLen)
{
    // ok a fixed size message, read it directly
    if(nMsgLen){ DoReadBody(nMsgLen); return; }

    // we need to get the length firstly
    auto fnDoneReadLen = [this](std::error_code stEC, size_t){
        if(stEC){ Stop(); return; }

        // now we have the length, read and make a package
        DoReadBody(m_BodyLen);
    };

    asio::async_read(m_Socket, asio::buffer(pData, nMsgLen), fnDoneReadLen);
}

void Session::Read()
{
    // I decided that Session only communicate with actor
    // then if the target address is not set there is no point to read
    //
    // TODO & TBD
    // this is a big change, the internal buffer mech is abondoned
    if(m_TargetAddress == Theron::Address::Null()){ return; }

    DoReadHC();
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
