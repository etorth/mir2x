#pragma once
#include <asio.hpp>
#include <functional>
#include <list>
#include <thread>

class Session;
class SessionAcceptor final
{
    public:
        SessionAcceptor(int);
    public:
        void Start();
        void StopSession(int);
        void Deliver(int, const Message &);
        void Dispatch(const Message &);
        void SetOnRecv(std::function<void(const Message &, Session &)>);

    private:
        void DoAccept();

    private:
        asio::io_service        m_IO;
        asio::ip::tcp::endpoint m_EndPoint;
        asio::ip::tcp::acceptor m_Acceptor;
        asio::ip::tcp::socket   m_Socket;
        std::list<Session *>    m_SessionList;
        int                     m_Count;
    private:
        std::thread            *m_Thread;
        std::function<void(const Message &, Session &)> m_OnRecvFunc;
};
