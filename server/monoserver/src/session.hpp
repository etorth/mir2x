/*
 * =====================================================================================
 *
 *       Filename: session.hpp
 *        Created: 09/03/2015 3:48:41 AM
 *  Last Modified: 04/29/2016 00:47:27
 *
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */
#pragma once
#include <tuple>
#include <cstdint>
#include <asio.hpp>
#include <queue>
#include <functional>

class SessionHub;
class Session
{
    public:
        Session(uint32_t,               // session id
                asio::ip::tcp::socket,  // socket
                SessionHub *,           // parent
                std::function<void(uint8_t, Session *)>);  // handler on header code

       ~Session();

    public:
       // TODO
       // Session class won't maintain the validation of pData!
       void Send(uint8_t nMsgHC, const uint8_t *pData,
               size_t nLen, const std::function<void()> &fnDone = []{})
       {
           bool bEmpty = m_SendQ.empty();
           m_SendQ.emplace(nMsgHC, pData, nLen, fnDone);

           if(bEmpty){
               DoSendHC();
           }
       }

       // helper functions
       void Send(uint8_t nMsgHC, const std::function<void()> &fnDone = []{})
       {
           Send(nMsgHC, nullptr, 0, fnDone);
       }

       // TODO
       // maybe I need make const T here
       template<typename T> void Send(
               uint8_t nMsgHC, const T &stMsgT, const std::function<void()> &fnDone = []{})
       {
           Send(nMsgHC, (const uint8_t *)(&stMsgT), sizeof(stMsgT), fnDone);
       }


    public:
       // read header code, operation is defined in constructor
       void ReadHC();
       // read data, operation is defined by functional argument
       void Read(size_t, std::function<void(uint8_t *, size_t)>);

    private:
       void DoSendHC();
       void DoSendBuf();
       void DoSendNext();

    public:
       uint32_t  ID()
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

       void Shutdown()
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
       uint32_t                m_ID;
       asio::ip::tcp::socket   m_Socket;
       SessionHub             *m_SessionHub;
       std::string             m_IP;
       int                     m_Port;
       uint8_t                 m_MessageHC;
       int                     m_ReadRequest;

    private:
       using SendTaskDesc = std::tuple<uint8_t,
             const uint8_t *, size_t, std::function<void()>>;

       std::function<void(uint8_t, Session *)> m_OperateFunc;
       std::queue<SendTaskDesc>                m_SendQ;
       std::vector<uint8_t>                    m_Buf;
};
