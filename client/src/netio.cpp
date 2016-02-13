/*
 * =====================================================================================
 *
 *       Filename: netio.cpp
 *        Created: 06/29/2015 7:18:27 PM
 *  Last Modified: 01/24/2016 20:37:37
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

#include <mutex>
#include <thread>
#include "message.hpp"
#include "netio.hpp"

NetIO::NetIO()
    : m_Resolver(m_IO)
    , m_Socket(m_IO)
{}

NetIO::~NetIO()
{}

bool NetIO::Connect(const std::string &szIP, const std::string &szPort)
{
    // auto fnConnect = [this](std::error_code stEC, asio::ip::tcp::resolver::iterator){
    //     if(!stEC){
    //         m_OK = true;
    //         DoReadHC();
    //     }else{
    //         m_Socket.close();
    //     }
    // };
    std::error_code stEC;
    m_Socket.connect(*(m_Resolver.resolve({szIP, szPort})), stEC);
    if(stEC){
        return false;
    }
    return true;
}

void NetIO::Stop()
{
    m_IO.post([this](){m_Socket.close();});
}

void NetIO::Send(uint8_t *pBuf, size_t nLen)
{
    // TODO
    // without mutex we have to allocate memory
    // since when leave this function pBuf may become invalid
    // or use internal buf operated with std::mutex
    //
    pNewBuf = new uint8_t[nLen];
    std::memcpy(pNewBuf, pBuf, nLen);
    auto fnWrite = [this, pNewBuf, nLen](){
        bool bEmpty = m_WriteQueue.empty();
        m_WriteQueue.emplace_back(pNewBuf, nLen);
        if(bEmpty){
            // if this is the only package then send it immediately
            // otherwise previously called DoSend() will continue to send
            DoSend();
        }
    };
    m_IO.post(fnWrite);
}

void NetIO::DoRead()
{
    auto fnRead = [this](std::error_code stEC, std::size_t){
        if(!stEC){
            if(m_WithContentFunc(m_MsgID)){
                size_t nLen = (m_ContentLengthFunc(m_MsgID));
                if(nLen > 0){
                    DoReadData(nLen);
                }else{
                    DoReadStream();
                }
            }else{
                // no content then consume it directly
                m_ConsumeFunc(m_MsgID, nullptr, 0);
            }
            // read again after processed the whole message
            DoRead();
        }else{
            m_Socket.close();
        }
    };
    asio::async_read(m_Socket, asio::buffer(&m_MsgID, 1), fnRead);
}

void NetIO::DoReadData(size_t nLength)
{
    uint8_t *pBuf = new uint8_t[nLength];
    auto fnRead = [this, pBuf, nLength](std::error_code stEC, std::size_t){
        if(!stEC){
            // successfully read fixed length data
            m_ConsumeFunc(m_MsgID, pBuf, nlength);
        }else{
            m_Socket.close();
        }
    };
    asio::async_read(m_Socket, asio::buffer(pBuf, nLength), fnRead);
}

void NetIO::DoReadStream()
{
    auto fnRead = [this](std::error_code stEC, std::size_t){
        if(!stEC){
            // successfully read fixed length data
           uint8_t *pBuf = new uint8_t[m_StreamBuf.size()];
           std::memcpy(pBuf, m_StreamBuf.data(), m_StreamBuf.size());
           m_ConsumeFunc(m_MsgID, pBuf, m_StreamBuf.size());
        }else{
            m_Socket.close();
        }
    };
    asio::async_read_until(m_Socket, m_StreamBuf, '\0', fnRead);
}

void NetIO::DoSend()
{
    auto fnSend = [this](std::error_code stEC, size_t){
        if(!stEC){
            // move forward for valid data
            m_ValidThrough = m_WriteQueue.front().first + m_WriteQueue.front().second;
            m_WriteQueue.pop_front();
            if(!m_WriteQueue.empty()){
                DoSend();
            }
        }else{
            m_Socket.close();
        }
    };

    // get buffer for the first message
    uint8_t *pBuf = m_Buf + m_WriteQueue.front().first;
    size_t   nLen = m_WriteQueue.front().second;

    asio::async_write(m_Socket, asio::buffer(pBuf, nLen), fnSend);
}
