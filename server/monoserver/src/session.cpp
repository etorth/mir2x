/*
 * =====================================================================================
 *
 *       Filename: session.cpp
 *        Created: 9/3/2015 3:48:41 AM
 *  Last Modified: 04/14/2017 12:38:53
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
    , m_FlushFlag(false)
    , m_NextQLock()
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
    assert(m_FlushFlag);
    assert(!m_CurrSendQ->empty());
    if(std::get<3>(m_CurrSendQ->front())){
        std::get<3>(m_CurrSendQ->front())();
    }

    m_CurrSendQ->pop();
    DoSendHC();
}

void Session::DoSendBuf()
{
    assert(m_FlushFlag);
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
    // when we are here
    // we should already have m_FlushFlag set as true
    assert(m_FlushFlag);

    // we check m_CurrSendQ and if it empty we swap with the pending queue
    // we move this swap thing to the handler in FlushSendQ()
    // means we only handle m_CurrSendQ in DoSendHC()
    // but which means after we done m_CurrSendQ we have to wait next FlushSendQ() to drive the send
    if(m_CurrSendQ->empty()){
        std::lock_guard<std::mutex> stLockGuard(m_NextQLock);
        if(m_NextSendQ->empty()){
            // neither queue contains pending packages
            // mark m_FlushFlag as no one accessing m_CurrSendQ and return
            m_FlushFlag = false;
            return;
        }else{
            // else we still need to access m_CurrSendQ 
            // keep m_FlushFlag to pervent other thread to call DoSendHC()
            std::swap(m_CurrSendQ, m_NextSendQ);
        }
    }

    assert(!m_CurrSendQ->empty());
    auto fnDoSendBuf = [this](std::error_code stEC, size_t){
        if(stEC){ Shutdown(); } else { DoSendBuf(); }
    };
    asio::async_write(m_Socket, asio::buffer(&(std::get<0>(m_CurrSendQ->front())), 1), fnDoSendBuf);
}

void Session::FlushSendQ()
{
    auto fnFlushSendQ = [this](){
        // m_CurrSendQ assessing should always be in the asio main loop
        // the Session::Send() should only access m_NextSendQ
        // 
        // then we don't have to put lock to protect m_CurrSendQ
        // but we need lock for m_NextSendQ, in child threads, in asio main loop
        //
        // but we need to make sure there is only one procedure in asio main loop accessing m_CurrSendQ
        // because packages in m_CurrSendQ are divided into  two parts: HC / Data 
        // one package only get erased after Data is sent
        // then  multiple procesdure in asio main loop may send HC / Data more than one time
        if(!m_FlushFlag){
            //  mark as current some one is accessing it
            //  we don't even need to make m_FlushFlag atomic since it's in one thread
            m_FlushFlag = true;
            DoSendHC();
        }
    };

    // FlushSendQ() is called by child threads only
    // so we prevent it from access m_CurrSendQ
    // instead everytime we post a handler to asio main loop
    //
    // this hurts the performance but make a better logic framework
    // we can also make m_FlushFlag atomic and use it to protect m_CurrSendQ
    m_Socket.get_io_service().post(fnFlushSendQ);
}
