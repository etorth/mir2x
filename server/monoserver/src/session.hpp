#pragma once
#include <asio.hpp>
#include <deque>
#include <functional>
#include "message.hpp"

class SessionManager;
class Session
{
    public:
        Session(asio::ip::tcp::socket, SessionAcceptor *, int);
        ~Session();

    public:
        int  ID();
        void Start();
        void Deliver(const Message &);
        void SetOnRecv(std::function<void(const Message &, Session &)>);

    public:
        const char *IP();
        int Port();

    private:
        void DoReadHeader();
        void DoReadBody();
        void DoWrite();
        void Confirm();

    private:
        std::function<void(const Message &, Session &)> m_OnReadMessage;

    private:
        asio::ip::tcp::socket  m_Socket;
        Message                m_Message;
        SessionAcceptor       *m_SessionAcceptor;
        std::deque<Message>    m_WriteMessageQueue;
        std::string            m_IP;
        int                    m_ID;
};
