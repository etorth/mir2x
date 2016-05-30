/*
 * =====================================================================================
 *
 *       Filename: session.hpp
 *        Created: 09/03/2015 03:48:41 AM
 *  Last Modified: 05/30/2016 01:12:40
 *
 *    Description: TODO & TBD
 *                 I have a decision, now class session *only* communicate with actor
 *                 advantages of it:
 *                 1. no need to assign new handler, since it's fixed
 *                 2. we only need to assign a new actor address when logic changes
 *                 3. every time when Session informs Actor, it comes with an fully
 *                    reveived message, not just an header code or something
 *
 *                 disadvantages:
 *                 1. now we need MessageSize(MessageHC) and MessageFixedSize(MessageHC)
 *                    to take care of the message structure, in other word, the logic of
 *                    messge structure now coupling with logic of session
 *                 2. all buffers can only be assigned by g_MemoryChunkPN
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
#include <Theron/Theron.h>

#include "syncdriver.hpp"

class SessionHub;
class Session: public SyncDriver
{
    private:
        using SendTaskDesc = std::tuple<uint8_t, const uint8_t *, size_t, std::function<void()>>;

    private:
        uint32_t                                m_ID;
        asio::ip::tcp::socket                   m_Socket;

        std::string                             m_IP;
        uint32_t                                m_Port;
        uint8_t                                 m_MessageHC;
        uint32_t                                m_BodyLen;
        Theron::Address                         m_TargetAddress;
        std::queue<SendTaskDesc>                m_SendQ;

    public:
        Session(uint32_t, asio::ip::tcp::socket);
        ~Session();

    public:
        // family of send facilities
        // TODO & TBD
        // Session class won't maintain the validation of pData!

        // send with a r-ref callback
        void Send(uint8_t nMsgHC, const uint8_t *pData, size_t nLen, std::function<void()> &&fnDone)
        {
            bool bEmpty = m_SendQ.empty();
            m_SendQ.emplace(nMsgHC, pData, nLen, std::move(fnDone));

            if(bEmpty){ DoSendHC(); }
        }

        // send with a const-ref callback
        void Send(uint8_t nMsgHC, const uint8_t *pData,
                size_t nLen, const std::function<void()> &fnDone)
        {
            Send(nMsgHC, pData, nLen, std::function<void()>(fnDone));
        }

        // send without a callback
        void Send(uint8_t nMsgHC, const uint8_t *pData, size_t nLen)
        {
            Send(nMsgHC, pData, nLen, std::function<void()>());
        }

        // send a message header code without a body
        void Send(uint8_t nMsgHC, std::function<void()> &&fnDone)
        {
            // ``r-ref itself is a l-ref", finally I understand it here
            Send(nMsgHC, nullptr, 0, std::move(fnDone));
        }

        // send a message header code without a body
        void Send(uint8_t nMsgHC, const std::function<void()> &fnDone)
        {
            Send(nMsgHC, nullptr, 0, std::function<void()>(fnDone));
        }

        // send a strcutre
        template<typename T> void Send(uint8_t nMsgHC,
                const T &stMsgT, const std::function<void()> &fnDone)
        {
            Send(nMsgHC, (const uint8_t *)(&stMsgT), sizeof(stMsgT), fnDone);
        }

        // send a strcutre
        template<typename T> void Send(uint8_t nMsgHC,
                const T &stMsgT, std::function<void()> &&fnDone)
        {
            Send(nMsgHC, (const uint8_t *)(&stMsgT), sizeof(stMsgT), std::move(fnDone));
        }

        // send a strcutre
        template<typename T> void Send(uint8_t nMsgHC, const T &stMsgT)
        {
            Send(nMsgHC, (const uint8_t *)(&stMsgT), sizeof(stMsgT));
        }

    private:
        void DoReadHC();
        void DoReadBody(size_t);

        void DoSendHC();
        void DoSendBuf();
        void DoSendNext();

    public:
        uint32_t ID()
        {
            return m_ID;
        }

        // return value:
        //  0 : OK
        //  1 : invalid arguments
        int Launch(const Theron::Address &rstAddr)
        {
            if(rstAddr == Theron::Address::Null()){ return 1; }
            m_TargetAddress = rstAddr;

            DoReadHC();
            return 0;
        }

        void Shutdown()
        {
            m_Socket.close();
            m_TargetAddress = Theron::Address::Null();
        }

        bool Valid()
        {
            return m_Socket.is_open() && m_TargetAddress != Theron::Address::Null();
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

        uint32_t Port()
        {
            return m_Port;
        }

        void Bind(const Theron::Address &rstAddr)
        {
            m_TargetAddress = rstAddr;
        }

        Theron::Address Bind()
        {
            return m_TargetAddress;
        }
};
