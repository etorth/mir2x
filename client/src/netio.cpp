/*
 * =====================================================================================
 *
 *       Filename: netio.cpp
 *        Created: 06/29/2015 7:18:27 PM
 *  Last Modified: 04/06/2016 18:12:49
 *
 *    Description: this class won't maintian buffer's validation
 *                 user should maintain it by themself
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

#include "netio.hpp"

NetIO::NetIO()
    : m_Resolver(m_IO)
    , m_Socket(m_IO)
    , m_MsgHC(0)
    , m_ReadCount(0)
{
}

NetIO::~NetIO()
{
    m_IO.stop();
}

void NetIO::StopIO()
{
    m_IO.post([this](){Close();});
}

// TODO
// I'm not sure whether I can make fnOperateHC as const reference
// since ...
void NetIO::RunIO(const char *szIP,
        const char * szPort, const std::function<void(uint8_t)> &fnOperateHC)
{
    // 1. push a connect request into the event queue
    // 2. start the event loop
    //
    // TODO
    //
    // I have no idea of that
    // what would happen if I post requests before m_IO.run() started???
    // or even before asio::async_connect()?
    //
    auto stIterator = m_Resolver.resolve({szIP, szPort});
    auto fnOnConnect = [this, fnOperateHC](std::error_code stEC, asio::ip::tcp::resolver::iterator){
        if(stEC){
            Close();
        }else{
            ReadHC(fnOperateHC);
        }
    };

    // push a connection request
    asio::async_connect(m_Socket, stIterator, fnOnConnect);

    // start the even loop and never return when OK
    m_IO.run();
}

void NetIO::Close()
{
    m_Socket.close();
}

void NetIO::Send(uint8_t nMsgHC)
{
    Send(nMsgHC, nullptr, 0);
}

// validation of pBuf is maintained by caller!
// just like asio::buffer()
void NetIO::Send(uint8_t nMsgHC, const uint8_t *pBuf, size_t nLen)
{
    auto fnSendHC = [this, nMsgHC, pBuf, nLen](){
        bool bEmpty = m_WQ.empty();
        m_WQ.emplace(nMsgHC, pBuf, nLen);
        if(bEmpty){
            // if this is the only package then send it immediately
            // otherwise previously called DoSend() will continue to send
            // this guarantee data will be sent in order
            DoSendHC();
        }
    };
    m_IO.post(fnSendHC);
}

// pBuf is provided by user, NetIO won't maintain its valid state
// but NetIO will provide a 1-byte buffer for nMsgHC
// TODO:
// Can NetIO provide an alway-valid buffer for user?
// seems it's possible
//
void NetIO::Read(uint8_t * pBuf,
        size_t nBufLen, const std::function<void()> &fnOperateBuf)
{
    // we suppress multi-read at one time
    // read request should be handled one by one
    assert(m_ReadCount == 0);

    auto fnOnReadBuf = [this, fnOperateBuf](std::error_code stEC, size_t){
        if(stEC){
            Close();
        }else{
            fnOperateBuf();
            m_ReadCount--;
        }
    };

    m_ReadCount++;
    asio::async_read(m_Socket, asio::buffer(pBuf, nBufLen), fnOnReadBuf);
}

void NetIO::ReadHC(const std::function<void(uint8_t)> &fnProcessHC)
{
    auto fnOnReadHC = [this, fnProcessHC](std::error_code stEC, size_t){
        if(!stEC){
            fnProcessHC(m_MsgHC);
        }else{
            Close();
        }
    };

    asio::async_read(m_Socket, asio::buffer(&m_MsgHC, 1), fnOnReadHC);
}

void NetIO::DoSendNext()
{
    m_WQ.pop();
    if(!m_WQ.empty()){
        DoSendHC();
    }
}

void NetIO::DoSendBuf()
{
    if(m_WQ.empty()){ return; }

    if(std::get<2>(m_WQ.front()) && std::get<1>(m_WQ.front())){
        auto fnDoSendValidBuf = [this](std::error_code stEC, size_t){
            if(stEC){
                Close();
            }else{
                DoSendNext();
            }
        };

        asio::async_write(m_Socket,
                asio::buffer((std::get<1>(m_WQ.front())),
                    std::get<2>(m_WQ.front())), fnDoSendValidBuf);
    }else{
        DoSendNext();
    }
}

void NetIO::DoSendHC()
{
    auto fnDoSendBuf = [this](std::error_code stEC, size_t){
        if(stEC){
            Close();
        }else{
            DoSendBuf();
        }
    };

    asio::async_write(m_Socket,
            asio::buffer(&(std::get<0>(m_WQ.front())), 1), fnDoSendBuf);
}
