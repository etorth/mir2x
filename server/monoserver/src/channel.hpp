/*
 * =====================================================================================
 *
 *       Filename: channel.hpp
 *        Created: 09/03/2015 03:48:41
 *    Description:
 *                 basic class for client-server communication
 *
 *                 for server -> client: Channel::Post(ServerMessage)
 *                 1. non-blocking
 *                 2. thread-safe
 *
 *                 for client -> server: Channel::ForwardActorMessage(MPK_NETPACKAGE)
 *                 1. use internal Dispatcher
 *                 2. won't register callback for forwarding
 *
 *                 need to take care of:
 *                 1. who is going to access this class?
 *                    the server threads and asio main loop thread will access it. For access from
 *                    server threads, need to make it thread safe. For access from the asio thread
 *                    since currently I only use one thread for asio, it's simpler
 *
 *                 2. Channel::Send(server_message) use internal memory pool to copy server_message
 *                    and post it to the asio main loop
 *
 *                 2. Channel::Forward(MPK_NETPACKAGE) use g_MemoryPN to forward actor message to
 *                    actor bound to it, and the actor should release the memroy back
 *
 *                 4. when using asio main loop we use a lot of
 *
 *                          asio::async_read(..., [this](){ ... });
 *
 *                    then registered handler will refer to *this* in asio main loop thread, but at
 *                    the same time we may call NetDriver::Shutdown(SessionID) to release one session
 *                    so this is releasing in server's actor threads.
 *
 *                    should be very careful of it since if we shutdown a session but asio loop thread
 *                    still have handlers referring it, it crashes. i.e.
 *
 *                    // in player actor thread
 *                    // player actor is sending PING every 1s
 *
 *                          g_NetDriver->Send(SessonID, SM_PING);
 *
 *                    // which eventually calls
 *
 *                          asio::io_service::post([this](){ Channel::MessageQueue::front() });
 *
 *                    // then there is a handler referring *this*
 *                     
 *
 *                    // in service core thread
 *                    // the cored decide to kill the session because of some abnormals
 *
 *                          g_NetDriver->Shutdown(SessionID);
 *
 *                    // which will do
 * 
 *                          delete SessionVector[SerssionID]
 *
 *                    // to free the session slot at index SessionID
 *                    // but if the SM_PING is not sent yet then this delete will cause UB
 *
 *                    to solve this issue
 *                    1. Channel has a SyncDriver
 *                    2. Channel is derived from std::enable_shared_from_this<Channel>
 *
 *                    then any time if we want to post a handler to asio main loop thread
 *                    referring the session itself we use
 *
 *                          asio::async_read(..., [pThis = shared_from_this()](){ ... })
 *
 *                    outside we use std::shared_ptr<Channel> and use reset() instead of the
 *                    raw pointer delete
 *
 *                    seems this is the standard way to do object new / delete in multi-thread
 *                    environment till now I know
 *
 *                    this class won't support re-connect
 *                    if connection is off we need to wait for the reconnection from client
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
#include <mutex>
#include <memory>
#include <cstdint>
#include <asio.hpp>
#include <functional>
#include <Theron/Theron.h>

#include "dispatcher.hpp"
#include "memorychunkpn.hpp"

class Channel final: public std::enable_shared_from_this<Channel>
{
    private:
        enum ChannStateType: int
        {
            CHANNTYPE_NONE    = 0,
            CHANNTYPE_RUNNING = 1,
            CHANNTYPE_STOPPED = 2,
        };

    private:
        // used by server threads, when server trying to post an send
        // it build a SendTask package by BuildTask() and post to the asio main loop thread
        struct SendTask
        {
            uint8_t HC;

            const uint8_t *Data;
            size_t         DataLen;

            std::function<void()> OnDone;

            // there are argument check when constructing SendTask
            // so put the implementation of the constructor in session.cpp
            SendTask(uint8_t, const uint8_t *, size_t, std::function<void()> &&);

            operator bool () const
            {
                return HC != 0;
            }

            static const SendTask &Null()
            {
                static SendTask stNullTask(0, nullptr, 0, [](){});
                return stNullTask;
            }
        };

    private:
        const uint32_t m_ID;

    private:
        Dispatcher m_Dispatcher;

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

    private:
        std::atomic<int> m_State;

    public:
        // server threads will call the constructor
        // in Channel::ChannBuild() called by std::make_shared<Channel>()
        Channel(uint32_t, asio::ip::tcp::socket);

    public:
        // asio thread will call the destructor
        // call destructor by handler destructor in asio main loop thread
        virtual ~Channel();

    public:
        uint32_t ID() const
        {
            return m_ID;
        }

        uint32_t Port() const
        {
            return m_Port;
        }

        uint32_t Delay() const
        {
            return m_Delay;
        }

        const char *IP() const
        {
            return m_IP.c_str();
        }

    public:
        // family of send facilities, called by server threads
        // Channel class accepts buffer and make a copy as SendTask internally
        // Channel class will do compression if needed based on message header code

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
        // called by server threads
        // use internal memory pool to create the task
        SendTask BuildTask(uint8_t, const uint8_t *, size_t, std::function<void()> &&);

    private:
        // interal functions isolated from server threads
        // following DoXXXFunc should only be invoked in asio main loop thread
        void DoReadHC();
        void DoReadBody(size_t, size_t);

        void DoSendHC();
        void DoSendBuf();
        void DoSendNext();

    private:
        // called by server threads
        // only called in Channel::Send(server_message)
        bool FlushSendQ();

    public:
        // called by asio main loop thread and server threads
        // it atomically set the session state, which would disable everything
        void Shutdown(bool);

    public:
        bool Launch(const Theron::Address &rstAddr);

    public:
        void BindActor(const Theron::Address &rstAddr)
        {
            // force messages forward to the new actor
            // use *post* rather than directly assignement
            // since asio main loop thread will access m_BindAddress

            // potential bug:
            // internal actor address won't get update immediately after this call
            auto fnBind = [rstAddr, this]()
            {
                m_BindAddress = rstAddr;
            };
            m_Socket.get_io_service().post(fnBind);
        }

    private:
        // called by asio main loop only
        // forward MPK_NETPACKAGE when one entire network message
        // data buffer allocated by global memory pool and released by actor receiving it
        bool ForwardActorMessage(uint8_t, const uint8_t *, size_t);
};
