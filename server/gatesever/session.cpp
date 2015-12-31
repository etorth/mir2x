#include "message.hpp"
#include "session.hpp"
#include "sessionacceptor.hpp"
#include "clientmessagedef.hpp"

Session::Session(asio::ip::tcp::socket stSocket, SessionAcceptor *pSessionAcceptor, int nID)
    : m_Socket(std::move(stSocket))
    , m_SessionAcceptor(pSessionAcceptor)
    , m_ID(nID)
{
    m_IP = m_Socket.remote_endpoint().address().to_string();
}

void Session::Start()
{
    Confirm();
    DoReadHeader();
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
