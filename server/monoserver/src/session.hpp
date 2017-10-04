/*
 * =====================================================================================
 *
 *       Filename: session.hpp
 *        Created: 09/03/2015 03:48:41 AM
 *  Last Modified: 10/03/2017 22:31:53
 *
 *    Description: actor <-> session <--- network ---> client
 *                 1. each session binds to an actor
 *                 2. session send AMNetPackage to actor by Forward()
 *
 *                 requirements:
 *                 1. session only sends fully received messages to actors
 *                 2. AMNetPackage contains a buffer allocated by g_MemoryPN to actors
 *                 3. use internal memory pool for send messages
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
#include "memorychunkpn.hpp"

class Session final: public SyncDriver
{
    private:
        struct SendTask
        {
            uint8_t HC;

            const uint8_t *Data;
            size_t         DataLen;

            std::function<void()> OnDone;

            // there are argument check when constructing SendTask
            // so put the implementation of the constructor in session.cpp
            SendTask(uint8_t, const uint8_t *, size_t, std::function<void()> &&);
        };

    private:
        const uint32_t m_ID;

    private:
        asio::ip::tcp::socket m_Socket;
        const std::string     m_IP;
        const uint32_t        m_Port;

    private:
        uint8_t         m_ReadHC;
        uint8_t         m_ReadLen[4];
        uint32_t        m_BodyLen;
        uint32_t        m_Delay;
        Theron::Address m_BindAddress;

    private:
        // 1. m_FlushFlag indicates there is procedure accessing m_CurrSendQ in asio main loop
        //    m_FlushFlag prevents more than one procedure from accessing m_CurrSendQ
        //
        // 2. m_NextQLock protect the pending queue: m_NextSendQ
        //    m_NextSendQ will be accessed in multi-threading manner
        bool       m_FlushFlag;
        std::mutex m_NextQLock;

    private:
        std::queue<SendTask>  m_SendQBuf0;
        std::queue<SendTask>  m_SendQBuf1;
        std::queue<SendTask> *m_CurrSendQ;
        std::queue<SendTask> *m_NextSendQ;

    private:
        // used for internal pending message storage
        // support multi-thread since external thread call Send which refers to it
        // then every session have an internal memory pool, too many?
        MemoryChunkPN<64, 256, 2> m_MemoryPN;

    public:
        Session(uint32_t, asio::ip::tcp::socket);
       ~Session();

    public:
        // family of send facilities
        // Session class accepts buffer and make a copy of it internally
        // Session class will do compression if needed based on message header code

        // send with a r-ref callback, this is the base of all send function
        // current implementation is based on double-queue method
        // one queue (Q1) is used for store new packages in parallel
        // the other (Q2) queue is used to send all packages in ASIO main loop
        //
        // then Q1 needs to be protected from data race
        // but for Q2 since it's only used in ASIO main loop, we don't need to protect it
        //
        // idea from: https://stackoverflow.com/questions/4029448/thread-safety-for-stl-queue
       bool Send(uint8_t nHC, const uint8_t *pData, size_t nLen, std::function<void()> &&fnDone);

    public:
        // send with a const-ref callback
        bool Send(uint8_t nHC, const uint8_t *pData, size_t nLen, const std::function<void()> &fnDone)
        {
            return Send(nHC, pData, nLen, std::function<void()>(fnDone));
        }

        // send without a callback
        bool Send(uint8_t nHC, const uint8_t *pData, size_t nLen)
        {
            return Send(nHC, pData, nLen, std::function<void()>());
        }

        // send a message header code without a body
        bool Send(uint8_t nHC, std::function<void()> &&fnDone)
        {
            // ``r-ref itself is a l-ref", finally I understand it here
            return Send(nHC, nullptr, 0, std::move(fnDone));
        }

        // send a message header code without a body
        bool Send(uint8_t nHC, const std::function<void()> &fnDone)
        {
            return Send(nHC, nullptr, 0, std::function<void()>(fnDone));
        }

        // send a strcutre
        template<typename T> bool Send(uint8_t nHC, const T &stMsgT, const std::function<void()> &fnDone)
        {
            return Send(nHC, (const uint8_t *)(&stMsgT), sizeof(stMsgT), fnDone);
        }

        // send a strcutre
        template<typename T> bool Send(uint8_t nHC, const T &stMsgT, std::function<void()> &&fnDone)
        {
            return Send(nHC, (const uint8_t *)(&stMsgT), sizeof(stMsgT), std::move(fnDone));
        }

        // send a strcutre
        template<typename T> bool Send(uint8_t nHC, const T &stMsgT)
        {
            return Send(nHC, (const uint8_t *)(&stMsgT), sizeof(stMsgT));
        }

    private:
        // following DoXXXFunc should only be invoked in asio main loop thread
        // since it may call Shutdown() and check Valid()

        void DoReadHC();
        void DoReadBody(size_t, size_t);

        void DoSendHC();
        void DoSendBuf();
        void DoSendNext();

    private:
        bool FlushSendQ();

    public:
        uint32_t ID() const
        {
            return m_ID;
        }

    public:
        // return value:
        //  0 : OK
        //  1 : invalid arguments
        int Launch(const Theron::Address &rstAddr)
        {
            if(!rstAddr){ return 1; }
            m_BindAddress = rstAddr;

            m_Socket.get_io_service().post([this](){ DoReadHC(); });
            return 0;
        }

        void Shutdown()
        {
            // for Session::Send() don't call this function
            // this function should only be called in asio main loop thread

            if(Valid()){
                Forward(MPK_BADSESSION, m_BindAddress);
                m_BindAddress = Theron::Address::Null();

                // if we call shutdown() here
                // we need to use try-catch since if connection has already
                // been broken, it throws exception

                // try{
                //     m_Socket.shutdown(asio::ip::tcp::socket::shutdown_both);
                // }catch(...){
                // }

                m_Socket.close();
            }
        }

        bool Valid() const
        {
            // data race possible with Shutdown()
            // call this funciton only in asio main loop thread
            return m_Socket.is_open() && m_BindAddress != Theron::Address::Null();
        }

        uint32_t Delay() const
        {
            return m_Delay;
        }

    public:
        const char *IP() const
        {
            return m_IP.c_str();
        }

        uint32_t Port() const
        {
            return m_Port;
        }

        void Bind(const Theron::Address &rstAddr)
        {
            m_BindAddress = rstAddr;
        }

        Theron::Address Bind() const
        {
            return m_BindAddress;
        }

    private:
        bool ForwardActorMessage(uint8_t, const uint8_t *, size_t);
};
