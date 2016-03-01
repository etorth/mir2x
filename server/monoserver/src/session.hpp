#pragma once
#include <tuple>
#include <cstdint>
#include <asio.hpp>
#include <queue>
#include <functional>

class SessionIO;
class Session
{
    public:
        Session(int,    // session id
                asio::ip::tcp::socket,  // socket
                SessionIO *,            // parent
                std::function<void(uint8_t, Session *)>);  // handler on header code

       ~Session();

    public:
       // read header code, operation is defined in constructor
       void ReadHC();
       // read data, operation is defined by functional argument
       void Read(size_t, std::function<void(uint8_t *, size_t)>);
       // send
       void Send(uint8_t, const uint8_t *, size_t);

       // short format
       void Send(uint8_t nMsgHC)
       {
           Send(nMsgHC, nullptr, 0);
       }
       // another helper
       template<typename T> void Send(uint8_t nMsgHC, T stMsgT)
       {
           Send(nMsgHC, (uint8_t *)(&stMsgT), sizeof(stMsgT));
       }


    private:
        void DoSendHC();
        void DoSendBuf();
        void DoSendNext();

    public:
        int  ID()
        {
            return m_ID;
        }

        void Launch()
        {
            ReadHC();
        }

        void Stop()
        {
            m_Socket.close();
        }

        bool Valid()
        {
            return m_Socket.is_open();
        }

    public:
        const char *IP()
        {
            // bug here: to_string create a temp std::string
            // and this object destructed when this line end
            // so c_str() is undefined
            //
            // return m_Socket.remote_endpoint().address().to_string().c_str();
            return m_IP.c_str();
        }

        int Port()
        {
            return m_Port;
        }

    private:
        int                     m_ID;
        asio::ip::tcp::socket   m_Socket;
        SessionIO              *m_SessionIO;
        std::string             m_IP;
        int                     m_Port;
        uint8_t                 m_MessageHC;
        int                     m_ReadRequest;

    private:
        std::function<void(uint8_t, Session *)>                  m_OperateFunc;
        std::queue<std::tuple<uint8_t, const uint8_t *, size_t>> m_SendQ;
        std::vector<uint8_t>                                     m_Buf;
};
