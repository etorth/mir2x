/*
 * =====================================================================================
 *
 *       Filename: messagemanager.cpp
 *        Created: 6/29/2015 7:18:27 PM
 *  Last Modified: 08/30/2015 6:26:10 PM
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
#include "messagemanager.hpp"
#include "configurationmanager.hpp"

MessageManager::MessageManager()
    : m_Resolver(m_IO)
    , m_Socket(m_IO)
{}

MessageManager::~MessageManager()
{}

MessageManager *GetMessageManager()
{
    static MessageManager messageManager;
    return &messageManager;
}

bool MessageManager::Init()
{
    return true;
}

void MessageManager::Release()
{}

void MessageManager::Start()
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

void MessageManager::Stop()
{
    m_IO.post([this](){m_Socket.close();});
    m_Thread->join();
}

void MessageManager::SendMessage(const Message &stMessage)
{
    // TODO
    // never call this function in any handler of asio lib!!!!
    // because the parameter stMessage will turn to be invalid
    //
    // if you have to, use static variable
    auto fnWrite = [this, stMessage](){
		// must capture by value
		// cz m_IO.post() will return soon
		// even we make stMessage as local, it'd destructed.
        bool bEmpty = m_WriteMessageQueue.empty();
        m_WriteMessageQueue.push_back(stMessage);
        if(bEmpty){
            DoSend();
        }
    };
    m_IO.post(fnWrite);
}

bool MessageManager::PollMessage(Message &stMessage)
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

void MessageManager::BatchHandleMessage(std::function<void(const Message &)> fnHandleMessage)
{
    if(fnHandleMessage){
        std::lock_guard<std::mutex> stLock(m_ReadMessageQueueMutex);
        for(auto &stMessage: m_ReadMessageQueue){
            fnHandleMessage(stMessage);
        }
        m_ReadMessageQueue.clear();
    }
}

void MessageManager::DoReadHeader()
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

void MessageManager::DoReadBody()
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

void MessageManager::DoSend()
{
    auto fnSend = [this](std::error_code stEC, std::size_t){
        if(!stEC){
            m_WriteMessageQueue.pop_front();
            if(!m_WriteMessageQueue.empty()){
                DoSend();
            }
        }else{
            m_Socket.close();
        }
    };
    auto &stMessage = m_WriteMessageQueue.front();
    asio::async_write(m_Socket, asio::buffer(stMessage.Data(), stMessage.Size()), fnSend);
}

void MessageManager::DoPush()
{
    std::lock_guard<std::mutex> stLock(m_ReadMessageQueueMutex);
    m_ReadMessageQueue.push_back(m_Message);
}
