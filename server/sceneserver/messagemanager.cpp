/*
 * =====================================================================================
 *
 *       Filename: messagemanager.cpp
 *        Created: 6/29/2015 7:18:27 PM
 *  Last Modified: 09/03/2015 8:23:00 PM
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

#include <asio.hpp>
#include <mutex>
#include <thread>
#include "message.hpp"
#include "messagemanager.hpp"
#include "networkconfigurewindow.hpp"
#include "sceneserver.hpp""

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
    extern NetworkConfigureWindow *g_NetworkConfigureWindow;
    auto stIterator = m_Resolver.resolve({
            g_NetworkConfigureWindow->GateServerIP(),
            std::to_string(g_NetworkConfigureWindow->GateServerPort()).c_str()});
    auto fnConnect = [this](std::error_code stEC, asio::ip::tcp::resolver::iterator){
        if(!stEC){
            DoReadHeader();
        }
        extern SceneServer *g_SceneServer;
        g_SceneServer->OnConnectSucceed();

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
    // never call this function in any handler of asio!
    // because the parameter will become invalid
    //
    // if you have to, use static variable
    auto fnWrite = [this, stMessage](){
        bool bEmpty = m_WriteMessageQueue.empty();
        m_WriteMessageQueue.push_back(stMessage);
        if(bEmpty){
            DoSend();
        }
    };
    m_IO.post(fnWrite);
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
            extern SceneServer *g_SceneServer;
            g_SceneServer->OnRecv(m_Message);
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
