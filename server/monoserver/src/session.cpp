#include "message.hpp"
#include "session.hpp"
#include "sessionmanager.hpp"

Session::Session(asio::ip::tcp::socket stSocket, SessionAcceptor *pSessionAcceptor, int nID)
    : m_Socket(std::move(stSocket))
    , m_SessionAcceptor(pSessionAcceptor)
    , m_ID(nID)
{
    m_IP = m_Socket.remote_endpoint().address().to_string();
}

void Session::Launch(fnOperateHC)
{
    ReadHC(fnOperateHC);
}





void Session::Deliver(const Message &stMessage)
{
    // TODO this is ok since when calling
    // stMessage is copied to queue immediately

    bool bEmpty = m_WriteMessageQueue.empty();
    m_WriteMessageQueue.push_back(stMessage);

    if(bEmpty){
        DoWrite();
    }
}

void Session::DoReadHeader()
{
    auto fnReadHeader = [this](std::error_code stEC, std::size_t){
        if(!stEC){
            DoReadBody();
        }else{
            m_SessionAcceptor->StopSession(m_ID);
        }
    };

    asio::async_read(m_Socket, asio::buffer(m_Message.Data(), m_Message.HeaderSize()), fnReadHeader);
}


void Session::DoReadBody()
{
    auto fnReadBody = [this](std::error_code stEC, std::size_t){
        if(!stEC){
            if(m_OnReadMessage){
                m_OnReadMessage(m_Message, *this);
            }
            DoReadHeader();
        }else{
            m_SessionAcceptor->StopSession(m_ID);
        }
    };

    asio::async_read(m_Socket, asio::buffer(m_Message.Body(), m_Message.BodySize()), fnReadBody);
}

void Session::DoWrite()
{
    auto fnDoWrite = [this](std::error_code stEC, std::size_t){
        if(!stEC){
            m_WriteMessageQueue.pop_front();
            if(!m_WriteMessageQueue.empty()){
                DoWrite();
            }
        }else{
            m_SessionAcceptor->StopSession(m_ID);
        }
    };

    auto &stMessage = m_WriteMessageQueue.front();
    asio::async_write(m_Socket, asio::buffer(stMessage.Data(), stMessage.Size()), fnDoWrite);
}

int Session::ID()
{
    return m_ID;
}

void Session::Confirm()
{
    Message stMessage;
    stMessage.SetIndex(CLIENTMT_CONNECTSUCCEED);
    Deliver(stMessage);
}

void Session::SetOnRecv(std::function<void(const Message &, Session &)> fnOnRecv)
{
    m_OnReadMessage = fnOnRecv;
}

const char *Session::IP()
{
    // bug here: to_string create a temp std::string
    // and this object destructed when this line end
    // so c_str() is undefined
    //
    // return m_Socket.remote_endpoint().address().to_string().c_str();
    return m_IP.c_str();
}

int Session::Port()
{
    return m_Socket.remote_endpoint().port();
}




    NetIO::NetIO()
    : m_Resolver(m_IO)
      , m_Socket(m_IO)
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

void NetIO::SetIO(const std::string &szIP,
        const std::string &szPort, std::function<void()> fnOperateHC)
{
    auto stIterator = m_Resolver.resolve({szIP, szPort});
    auto fnOnConnect = [this, fnOperateHC](std::error_code stEC, asio::ip::tcp::resolver::iterator){
        if(stEC){
            Close();
        }else{
            DoReadHC(fnOperateHC);
        }
    };

    asio::async_connect(m_Socket, stIterator, fnOnConnect);
}

void NetIO::RunIO()
{
    // start the event loop and never return when in normal operation
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

void Session::Read(int nLen, std::function<void()> fnOperateBuf)
{
    auto fnOnReadBuf = [this, fnOperateBuf](std::error_code stEC, size_t){
        if(stEC){
            Close();
        }else{
            fnOperateBuf();
        }
    };
    asio::async_read(m_Socket,
            asio::buffer(pBuf, nBufLen), fnOnReadBuf);
}


void Session::ReadHC(std::function<void(uint8_t, Session *)> fnProcessHC)
{
    auto fnOnReadHC = [this, fnProcessHC](std::error_code stEC, size_t){
        if(stEC){
            Close();
        }else{
            fnProcessHC(m_MessageHC, this);
        }
    };

    asio::async_read(m_Socket, asio::buffer(&m_MessageHC, 1), fnOnReadHC);
}


// for read data, class session will maintain its validation
void Session::Read(int nLen, std::function<void(uint8_t *, int)> fnProcessData)
{
    uint8_t *pData = AllocateBuf(nLen);

    auto fnOnReadData = [this, nLen, fnOperateData, pData](std::error_code stEC, size_t){
        if(stEC){
            Close();
        }else{
            fnProcessData(pData, nLen);
        }
    };

    asio::async_read(m_Socket, asio::buffer(pData, nLen), fnOnReadData);
}

void Session::ReadHC(std::function<void(uint8_t)> fnProcessHC)
{
    auto fnOnReadHC = [this, fnProcessHC](std::error_code stEC, size_t){
        if(!stEC){
            fnProcessHC(m_SessionID, m_UID, m_MessageHC, m_Socket);
        }else{
            Close();
        }
    };

    asio::async_read(m_Socket, asio::buffer(&m_MessageHC, 1), fnOnReadHC);
}

void NetIO::ReadHC(std::function<void(uint8_t)> fnProcessHC)
{
    static uint8_t nMsgHC;
    auto fnOnReadHC = [this, fnProcessHC, &nMsgHC](std::error_code stEC, size_t){
        if(!stEC){
            fnProcessHC(nMsgHC);
        }else{
            Close();
        }
    };

    asio::async_read(m_Socket, asio::buffer(&nMsgHC, 1), fnOnReadHC);
}



void NetIO::DoSendNext()
{
    m_WQ.pop_front();
    if(!m_WQ.empty()){
        DoSendHC();
    }
}

void NetIO::DoSendBuf()
{
    if(m_WQ.empty){ return; }

    if(std::get<0>(m_WQ.front()) && std::get<1>(m_WQ.front())){
        auto fnDoSendValidBuf = [this](std::error_code stEC, size_t){
            if(stEC){
                Close();
            }else{
                DoSendNext();
            }
        };

        asio::async_write(m_Socket,
                asio::buffer(&(std::get<1>(m_WQ.front())), std::get<2>(m_WQ.front())), fnDoSendBuf);
    }else{
        DoSendNext();
    }
}

void NetIO::DoSendHC()
{
    auto fnDoSendBuf = [this, fnSendBuf](std::error_code stEC, size_t){
        if(stEC){
            Close();
        }else{
            DoSendBuf();
        }
    };

    asio::async_write(m_Socket,
            asio::buffer(&(std::get<0>(m_WQ.front())), 1), fnDoSendBuf);
}

void Session

void NetIO::ReadHC(std::function<void(uint8_t)> fnProcessHC)
{
    static uint8_t nMsgHC;
    auto fnOnReadHC = [this, fnProcessHC, &nMsgHC](std::error_code stEC, size_t){
        if(!stEC){
            fnProcessHC(nMsgHC);
        }else{
            Close();
        }
    };

    asio::async_read(m_Socket, asio::buffer(&nMsgHC, 1), fnOnReadHC);
}

void Session::Read()
