#pragma once
#include <asio.hpp>
#include "message.hpp"
#include <deque>
#include <mutex>
#include <functional>
#include <thread>

class MessageManager
{
    private:
		MessageManager();
        ~MessageManager();

    private:
        asio::io_service        m_IO;
        asio::ip::tcp::resolver m_Resolver;
        asio::ip::tcp::socket   m_Socket;
        asio::ip::tcp::endpoint m_EndPoint;

    private:
        std::thread            *m_Thread;
        std::deque<Message>     m_WriteMessageQueue;
    private:
        Message     m_Message;

    public:
        bool Init();
        void Release();
        void Start();
        void Stop();
        void SendMessage(const Message &);

    private:
        void DoReadHeader();
        void DoReadBody();
        void DoSend();

    public:
        friend MessageManager *GetMessageManager();
};
