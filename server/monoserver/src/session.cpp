#include <cassert>
#include "session.hpp"
#include "sessionio.hpp"

Session::Session(int nSessionID,
        asio::ip::tcp::socket stSocket,
        SessionIO *pSessionIO,
        std::function<void(uint8_t, Session *)> fnOperateHC)
    : m_ID(nSessionID)
    , m_Socket(std::move(stSocket))
    , m_SessionIO(pSessionIO)
    , m_IP()
    , m_Port(0)
    , m_MessageHC(0)
    , m_ReadRequest(0)
    , m_OperateFunc(fnOperateHC)
{
    m_IP   = m_Socket.remote_endpoint().address().to_string();
    m_Port = m_Socket.remote_endpoint().port();

    m_Buf.resize(512, 0);
}

Session::~Session()
{
    Stop();
}

void Session::ReadHC()
{
    auto fnOnReadHC = [this](std::error_code stEC, size_t){
        if(stEC){
            Stop();
        }else{
            m_OperateFunc(m_MessageHC, this);
        }
    };

    asio::async_read(m_Socket, asio::buffer(&m_MessageHC, 1), fnOnReadHC);
}

// for read data, we use the internal buffer
// this means at any time, they could only be *one* read-request in the io-queue
// otherwise data in buffer will be destroied
void Session::Read(size_t nLen, std::function<void(uint8_t *, size_t)> fnProcessData)
{
    m_ReadRequest++;
    assert(m_ReadRequest == 1);

    if(nLen > m_Buf.size()){
        m_Buf.resize(nLen);
    }

    auto fnOnReadData = [this, nLen, fnProcessData](std::error_code stEC, size_t){
        if(stEC){
            Stop();
        }else{
            m_ReadRequest--;
            assert(m_ReadRequest == 0);
            fnProcessData(&(m_Buf[0]), nLen);
        }
    };

    asio::async_read(m_Socket, asio::buffer(&(m_Buf[0]), nLen), fnOnReadData);
}

// we assume there always be at least one SendTaskDesc
// since it's a callback after a send task done
void Session::DoSendNext()
{
    // 1. invoke the callback
    std::get<3>(m_SendQ.front())();
    // 2. remove the front
    m_SendQ.pop();
    // 3. do send if we have more work
    if(!m_SendQ.empty()){
        DoSendHC();
    }
}

void Session::DoSendBuf()
{
    if(m_SendQ.empty()){ return; }

    if(std::get<1>(m_SendQ.front()) && (std::get<2>(m_SendQ.front()) > 0)){
        auto fnDoSendValidBuf = [this](std::error_code stEC, size_t){
            if(stEC){
                Stop();
            }else{
                DoSendNext();
            }
        };

        asio::async_write(m_Socket,
                asio::buffer(&(std::get<1>(m_SendQ.front())), std::get<2>(m_SendQ.front())), fnDoSendValidBuf);
    }else{
        DoSendNext();
    }
}

void Session::DoSendHC()
{
    auto fnDoSendBuf = [this](std::error_code stEC, size_t){
        if(stEC){
            Stop();
        }else{
            DoSendBuf();
        }
    };

    asio::async_write(m_Socket, asio::buffer(&(std::get<0>(m_SendQ.front())), 1), fnDoSendBuf);
}
