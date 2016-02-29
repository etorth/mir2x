#pragma once
#include <asio.hpp>
#include <functional>
#include <list>
#include <thread>

class Session;
class SessionIO final
{
    public:
        SessionIO(int);

    public:
        void Start(std::function<void()>);
        void StopSession(int);

    private:
        void DoAccept();

    private:
        int AllocateID();

    private:
        asio::io_service        m_IO;
        asio::ip::tcp::endpoint m_EndPoint;
        asio::ip::tcp::acceptor m_Acceptor;
        asio::ip::tcp::socket   m_Socket;

    private:
        std::unordered_map<int, Session *>  m_SessionHub;

    private:
        std::thread *m_Thread;
};
