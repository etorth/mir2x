/*
 * =====================================================================================
 *
 *       Filename: session.hpp
 *        Created: 09/03/2015 03:48:41 AM
 *  Last Modified: 04/14/2017 00:11:07
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
#include <queue>
#include <tuple>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <asio.hpp>
#include <functional>
#include <Theron/Theron.h>

#include "syncdriver.hpp"

class SessionHub;
class Session: public SyncDriver
{
    private:
        using SendTaskDesc = std::tuple<uint8_t, const uint8_t *, size_t, std::function<void()>>;

    private:
        const uint32_t m_ID;

    private:
        asio::ip::tcp::socket m_Socket;
        const std::string     m_IP;
        const uint32_t        m_Port;

    private:
        uint8_t         m_MessageHC;
        uint32_t        m_BodyLen;
        Theron::Address m_BindAddress;

    private:
        // 1. m_Lock protect the pending queue: m_NextSendQ
        //    m_NextSendQ will be accessed in multi-threading way
        //
        // 2. m_SendFlag to indicate current the m_CurrSendQ is sending packages
        //    m_CurrSendQ will only be accessed in ASIO main loop
        //    and it only allow one procedure to access it
        std::mutex                m_Lock;
        std::atomic<int>          m_SendFlag;
        std::queue<SendTaskDesc>  m_SendQBuf0;
        std::queue<SendTaskDesc>  m_SendQBuf1;
        std::queue<SendTaskDesc> *m_CurrSendQ;
        std::queue<SendTaskDesc> *m_NextSendQ;

    public:
        Session(uint32_t, asio::ip::tcp::socket);
       ~Session();

    public:
        // family of send facilities
        // Session class won't maintain the validation of pData!

        // send with a r-ref callback, this is the base of all send function
        // current implementation is based on double-queue method
        // one queue (Q1)is used for store new packages in parallel
        // the other (Q2) queue is used to send all packages in ASIO main loop
        //
        // then Q1 needs to be protected from data race
        // but for Q2 since it's only used in ASIO main loop, we don't need to protect it
        //
        // idea from: https://stackoverflow.com/questions/4029448/thread-safety-for-stl-queue
        void Send(uint8_t nMsgHC, const uint8_t *pData, size_t nLen, std::function<void()> &&fnDone)
        {
            // 1. push packages to the pending queue
            {
                std::lock_guard<std::mutex> stLockGuard(m_Lock);
                m_NextSendQ->emplace(nMsgHC, pData, nLen, std::move(fnDone));
            }

            // 2. try to send if possible
            //    DoSendHC() will finish current sending queue first
            //    then when sending queue is done, it will try the pending queue
            //    if both queue are empty, it will return
            //
            //    every time after we push a new pending package
            //    we should try to call DoSendHC() to drive the whole sending process
            //    but there could only be one sending process, otherwise we get data race
            switch(m_SendFlag.exchange(1)){
                case 0:
                    {
                        DoSendHC();
                        return;
                    }
                default:
                    {
                        // think about that could there be any possibilities that m_SendFlag is 1
                        // but there is actually no one accessing m_CurrSendQ ???
                        return;
                    }
            }
        }

        // send with a const-ref callback
        void Send(uint8_t nMsgHC, const uint8_t *pData, size_t nLen, const std::function<void()> &fnDone)
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
        template<typename T> void Send(uint8_t nMsgHC, const T &stMsgT, const std::function<void()> &fnDone)
        {
            Send(nMsgHC, (const uint8_t *)(&stMsgT), sizeof(stMsgT), fnDone);
        }

        // send a strcutre
        template<typename T> void Send(uint8_t nMsgHC, const T &stMsgT, std::function<void()> &&fnDone)
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
        uint32_t ID() const
        {
            return m_ID;
        }

        // return value:
        //  0 : OK
        //  1 : invalid arguments
        int Launch(const Theron::Address &rstAddr)
        {
            if(!rstAddr){ return 1; }
            m_BindAddress = rstAddr;
            DoReadHC();
            return 0;
        }

        void Shutdown()
        {
            m_Socket.close();
            m_BindAddress = Theron::Address::Null();
        }

        bool Valid()
        {
            return m_Socket.is_open() && m_BindAddress != Theron::Address::Null();
        }

    public:
        const char *IP()
        {
            return m_IP.c_str();
        }

        uint32_t Port()
        {
            return m_Port;
        }

        void Bind(const Theron::Address &rstAddr)
        {
            m_BindAddress = rstAddr;
        }

        Theron::Address Bind()
        {
            return m_BindAddress;
        }
};
