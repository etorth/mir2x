#pragma once
#include <asio.hpp>
#include "message.hpp"
#include <deque>
#include <mutex>
#include <functional>
#include <thread>

class NetIO
{
    private:
		NetIO();
        ~NetIO();

    private:
        asio::io_service        m_IO;
        asio::ip::tcp::resolver m_Resolver;
        asio::ip::tcp::socket   m_Socket;
        asio::ip::tcp::endpoint m_EndPoint;

    private:
        std::thread            *m_Thread;
        std::mutex              m_ReadMessageQueueMutex;
        std::deque<Message>     m_ReadMessageQueue;
        std::deque<Message>     m_WriteMessageQueue;
    private:
        Message     m_Message;

    public:
        bool Init();
        void Release();
        void Start();
        void Stop();
        void Send(uint8_t *, size_t);
        void SendMessage(const Message &);
        bool PollMessage(Message &);

    public:
        void BatchHandleMessage(std::function<void(const Message &)>);

    private:
        void DoReadHeader();
        void DoReadBody();
        void DoSend();
        void DoPush();

    public:
        void Send(uint8_t *, size_t);

    public:
        void Send(uint8_t chMsg)
        {
            Send(&chMsg, 1);
        }

        template<typename MT>
        void Send(const MT &stMT)
        {
            Send((uint8_t *)(&stMT), sizeof(stMT));
        }
};
