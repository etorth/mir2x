/*
 * =====================================================================================
 *
 *       Filename: netio.cpp
 *        Created: 6/29/2015 7:18:27 PM
 *  Last Modified: 01/16/2016 09:47:40
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
#include "configurationmanager.hpp"

NetIO::NetIO()
    : m_Resolver(m_IO)
    , m_Socket(m_IO)
{}

NetIO::~NetIO()
{}

NetIO *GetMessageManager()
{
    static NetIO messageManager;
    return &messageManager;
}

bool NetIO::Init()
{
    return true;
}

void NetIO::Release()
{}

void NetIO::Start()
{
    auto stIterator = m_Resolver.resolve({GetConfigurationManager()->GetString("Root/Server/IP"),
            GetConfigurationManager()->GetString("Root/Server/Port")});
    auto fnConnect = [this](std::error_code stEC, asio::ip::tcp::resolver::iterator){
        if(!stEC){
            DoReadHeader();
        }
    };
    asio::async_connect(m_Socket, stIterator, fnConnect);
    m_Thread = new std::thread([this](){ m_IO.run(); });
}

void NetIO::Stop()
{
    m_IO.post([this](){m_Socket.close();});
    m_Thread->join();
}

void NetIO::Send(uint8_t *pBuf, size_t nLen)
{
    // TODO
    // never call this function in any handler of asio lib!!!!
    // because the parameter pBuf will turn to be invalid

    pNewBuf = new uint8_t[nLen];
    std::memcpy(pNewBuf, pBuf, nLen);
    auto fnWrite = [this, pNewBuf, nLen](){
        bool bEmpty = m_WriteQueue.empty();
        m_WriteQueue.emplace_back(pNewBuf, nLen);
        if(bEmpty){
            DoSend();
        }
    };
    m_IO.post(fnWrite);
}

bool NetIO::PollMessage(Message &stMessage)
{
    std::lock_guard<std::mutex> stLock(m_ReadMessageQueueMutex);
    if(m_ReadMessageQueue.empty()){
        return false;
    }else{
        stMessage = m_ReadMessageQueue.front();
        m_ReadMessageQueue.pop_front();
        return true;
    }
}

void NetIO::BatchHandleMessage(std::function<void(const Message &)> fnHandleMessage)
{
    if(fnHandleMessage){
        std::lock_guard<std::mutex> stLock(m_ReadMessageQueueMutex);
        for(auto &stMessage: m_ReadMessageQueue){
            fnHandleMessage(stMessage);
        }
        m_ReadMessageQueue.clear();
    }
}

void NetIO::Read(uint8_t *pBuf, size_t nLen)
{
}

void NetIO::DoReadHeader()
{
    auto fnRead = [this](std::error_code stEC, std::size_t){
        if(!stEC){
            DoReadBody();
        }else{
            m_Socket.close();
        }
    };
    asio::async_read(m_Socket, asio::buffer(m_Message.Data(), m_Message.HeaderSize()), fnRead);
}

void NetIO::DoReadBody()
{
    auto fnRead = [this](std::error_code stEC, std::size_t) {
        if(!stEC){
            DoPush();
            DoReadHeader();
        }else{
            m_Socket.close();
        }
    };
    asio::async_read(m_Socket, asio::buffer(m_Message.Body(), m_Message.BodySize()), fnRead);
}

void NetIO::DoSend()
{
    auto fnSend = [this](std::error_code stEC, size_t){
        if(!stEC){
            delete [] m_WriteQueue.front().first;
            m_WriteQueue.pop_front();
            if(!m_WriteQueue.empty()){
                DoSend();
            }
        }else{
            m_Socket.close();
        }
    };
    asio::async_write(m_Socket,
            asio::buffer(m_WriteQueue.front().first, m_WriteQueue.front().second), fnSend);
}

void NetIO::DoPush()
{
    std::lock_guard<std::mutex> stLock(m_ReadMessageQueueMutex);
    m_ReadMessageQueue.push_back(m_Message);
}
