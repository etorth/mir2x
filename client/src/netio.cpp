/*
 * =====================================================================================
 *
 *       Filename: netio.cpp
 *        Created: 06/29/2015 7:18:27 PM
 *  Last Modified: 06/13/2016 22:19:59
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

#include <cassert>
#include "netio.hpp"

NetIO::NetIO()
    : m_IO()
    , m_Resolver(m_IO)
    , m_Socket(m_IO)
    , m_MsgHC(0)
{}

NetIO::~NetIO()
{
    m_IO.stop();
}

// TODO
// I'm not sure whether I can make fnOperateHC as const reference
// since ...
void NetIO::RunIO(const char *szIP, const char * szPort, const std::function<void(uint8_t)> &fnOperateHC)
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


void NetIO::StopIO()
{
    m_IO.post([this](){Close();});
}

void NetIO::Close()
{
    m_Socket.close();
}

void NetIO::Send(uint8_t nMsgHC, const uint8_t *pBuf, size_t nLen, std::function<void()> &&fnDone)
{
    auto fnSendHC = [this, nMsgHC, pBuf, nLen, fnDone = std::move(fnDone)](){
        bool bEmpty = m_WQ.empty();
        m_WQ.emplace(nMsgHC, pBuf, nLen, std::move(fnDone));
        if(bEmpty){
            // if this is the only package then send it immediately, otherwise previously called
            // DoSend() will continue to send this guarantee data will be sent in order
            DoSendHC();
        }
    };

    m_IO.post(fnSendHC);
}

// this function read data to fullfil a buffer provided, if the provided buffer is nullptr, then we have
// to new a buffer to store result for the callback, after the callback we need to free it, this helps
// to handle more than one read request at one time in the ASIO queue
//          Read(nullptr,  2, fnOperate1);
//          Read(nullptr, 16, fnOperate2);
//          Read(nullptr, 24, fnOperate3);
// if we use internal buffer, we have to do it like this
//          Read(xxx, 2, [](xxx, 2){ Read(xxx, 16); });
//          
void NetIO::Read(uint8_t *pBuf, size_t nBufLen, std::function<void(const uint8_t *, size_t)> &&fnOperateBuf)
{
    // TODO: we need to report this error actually
    assert(nBufLen);

    bool bDelete = (pBuf ? false : true);
    uint8_t *pNewBuf = (pBuf ? pBuf : (new uint8_t[nBufLen]));

    auto fnOnReadBuf = [this, pNewBuf, nBufLen, bDelete, fnOperateBuf = std::move(fnOperateBuf)](std::error_code stEC, size_t){
        if(stEC){
            Close();
        }else{
            // 1. invoke the callback
            if(fnOperateBuf){
                fnOperateBuf(pNewBuf, nBufLen);
            }

            // 2. delete the buffer if needed
            if(bDelete){ delete [] pNewBuf; }
        }
    };

    asio::async_read(m_Socket, asio::buffer(pNewBuf, nBufLen), fnOnReadBuf);
}

void NetIO::ReadHC(std::function<void(uint8_t)> &&fnProcessHC)
{
    auto fnOnReadHC = [this, fnProcessHC = std::move(fnProcessHC)](std::error_code stEC, size_t){
        if(stEC){
            Close();
        }else{
            if(fnProcessHC){
                fnProcessHC(m_MsgHC);
            }
        }
    };

    asio::async_read(m_Socket, asio::buffer(&m_MsgHC, 1), fnOnReadHC);
}

void NetIO::DoSendNext()
{
    // 1. pop the old message, callback has been invoked
    m_WQ.pop();

    // 2. if there isn't any message, stop here
    //    so when call Send(), we need to check whether it's empty
    if(!m_WQ.empty()){ DoSendHC(); }
}

void NetIO::DoSendBuf()
{
    // can't happen actually since DoSendBuf() called always by DoSendHC()
    if(m_WQ.empty()){ return; }

    if(std::get<1>(m_WQ.front()) && std::get<2>(m_WQ.front())){
        auto fnDoSendValidBuf = [this](std::error_code stEC, size_t){
            if(stEC){
                Close();
            }else{
                // 1. invoke the callback
                if(std::get<3>(m_WQ.front())){
                    std::get<3>(m_WQ.front())();
                }
                // 2. send the next one
                DoSendNext();
            }
        };

        asio::async_write(m_Socket, asio::buffer((std::get<1>(m_WQ.front())), std::get<2>(m_WQ.front())), fnDoSendValidBuf);
    }else{
        // 1. invoke the callback
        if(std::get<3>(m_WQ.front())){
            std::get<3>(m_WQ.front())();
        }
        // 2. send the next one
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

    asio::async_write(m_Socket, asio::buffer(&(std::get<0>(m_WQ.front())), 1), fnDoSendBuf);
}

void NetIO::InitIO(const char *szIP, const char * szPort, const std::function<void(uint8_t)> &fnOperateHC)
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

    // now we need to keep polling it
}

void NetIO::PollIO()
{
    m_IO.poll();
}
