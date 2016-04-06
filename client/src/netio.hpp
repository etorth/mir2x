#pragma once
#include <queue>
#include <asio.hpp>
#include <functional>
#include <type_traits>

class NetIO final
{
    private:
        asio::io_service        m_IO;
        asio::ip::tcp::resolver m_Resolver;
        asio::ip::tcp::socket   m_Socket;

    private:
        // send queue
        // this provides data stream in send order
        std::queue<std::tuple<
            uint8_t,          // Message HC
            const uint8_t *,  // Message buf, NetIO won't maintain its validation
            size_t>> m_WQ;

    public:
        NetIO();
        ~NetIO();

    public:
        void RunIO(const char *, const char *, std::function<void(uint8_t)>);
        void StopIO();

    public:
        // send a body-less head code
        void Send(uint8_t);
        // send a HC and its body buffer
        // user should maintain the validation of the buffer
        void Send(uint8_t, const uint8_t *, size_t);

        // 1. template T couldn't be copy since NetIO won't maintain
        //    the send buffer
        // 2. try to use std::remove_reference_t<T>
        // 3. try to disable user to pass a rvalue here
        template<typename T> void Send(uint8_t nMsgHC, T &stMsg)
        {
            Send(nMsgHC, (const uint8_t *)(&stMsg), sizeof(stMsg));
        }

        // read a HC
        // user should provide the method to consume the HC
        void ReadHC(std::function<void(uint8_t)>);

        // read specified length of data to the buffer
        // user should maintain the validation of the buffer
        void Read(uint8_t *, size_t, std::function<void()>);

        template<typename T> void Read(T stMsg, std::function<void()> fnOperateBuf)
        {
            Read((uint8_t *)(&stMsg), sizeof(stMsg), fnOperateBuf);
        }

    private:
        void Close();

        void DoSendHC();
        void DoSendBuf();
        void DoSendNext();

    private:
        uint8_t m_MsgHC;
};
