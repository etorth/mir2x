/*
 * =====================================================================================
 *
 *       Filename: netio.cpp
 *        Created: 6/29/2015 7:18:27 PM
 *  Last Modified: 01/23/2016 03:51:57
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


void NetIO::ReadHC()
{

}

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
    m_Thread->join();
}

void NetIO::Send(uint8_t *pBuf, size_t nLen)
{
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


void NetIO::DoReadHC()
{
    auto fnRead = [this](std::error_code stEC, std::size_t){
        if(!stEC){
            // successfully read one HC
            if(m_HC){
                // fixed size message, only 1 bit
                if(m_OnReadHC){
                    m_OnReadHC(m_HC);
                }
                DoReadHC();
            }else{
                DoReadSubHC();
            }
        }else{
            m_Socket.close();
        }
    };
    asio::async_read(m_Socket, asio::buffer(&m_HC, 1), fnRead);
}

void NetIO::DoReadSubHC()
{
    std::assert(m_HC == 0);
    auto fnRead = [this](std::error_code stEC, std::size_t){
        if(!stEC){
            // successfully read one SubHC
            if(m_HC){
                // short data stream with length m_HC
                DoReadData((size_t)m_HC);
            }else{
                DoReadStream();
            }
        }else{
            m_Socket.close();
        }
    };
    asio::async_read(m_Socket, asio::buffer(&m_SubHC, 1), fnRead);
}

void NetIO::DoReadData(size_t nLength)
{
    std::assert(m_HC == 0 && m_SubHC != 0);

    void *pBuf = new char[(size_t)m_SubHC];
    auto fnRead = [this](std::error_code stEC, std::size_t){
        if(!stEC){
            // successfully read fixed length data
            if(m_OnReadData){
                m_OnReadData(pBuf, m_SubHC);
            }
            ReadHC();
        }else{
            m_Socket.close();
        }
    };

    asio::async_read(m_Socket, asio::buffer(pBuf, m_SubHC), fnRead);
}

void NetIO::DoReadStream()
{
    std::assert(m_HC == 0 && m_SubHC == 0);

    auto fnRead = [this](std::error_code stEC, std::size_t){
        if(!stEC){
            // successfully read fixed length data
            if(m_OnReadStream){
                void *pBuf = new char[m_StreamBuf.size()];
                std::memcpy(pBuf, m_StreamBuf.data(), m_StreamBuf.size());
                m_OnReadStream(pBuf, m_OnReadStrea.size());
            }
            ReadHC();
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
