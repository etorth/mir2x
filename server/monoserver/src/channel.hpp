/*
 * =====================================================================================
 *
 *       Filename: channel.hpp
 *        Created: 09/03/2015 03:48:41
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
#include <mutex>
#include <memory>
#include <cstdint>
#include <asio.hpp>
#include <functional>
#include <Theron/Theron.h>

#include "dispatcher.hpp"
#include "channpackq.hpp"

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
        const uint32_t m_ID;

    private:
        std::atomic<int> m_State;

    private:
        Dispatcher m_Dispatcher;

    private:
        asio::ip::tcp::socket m_Socket;
        const std::string     m_IP;
        const uint32_t        m_Port;

    private:
        // for read channel packets
        // only asio main loop accesses these fields
        uint8_t  m_ReadHC;
        uint8_t  m_ReadLen[4];
        uint32_t m_BodyLen;

    private:
        std::vector<uint8_t> m_ReadBuf;
        std::vector<uint8_t> m_DecodeBuf;

    private:
        Theron::Address m_BindAddress;

    private:
        // for post channel packets
        // server thread and asio main loop accesses these feilds
        //
        // 1. m_FlushFlag indicates there is procedure accessing m_CurrSendQ in asio main loop
        //    m_FlushFlag prevents more than one procedure from accessing m_CurrSendQ
        //
        // 2. m_NextQLock protect the pending queue: m_NextSendQ
        //    m_NextSendQ will be accessed by server thread to push packets in
        bool       m_FlushFlag;
        std::mutex m_NextQLock;

    private:
        ChannPackQ  m_SendPackQ0;
        ChannPackQ  m_SendPackQ1;
        ChannPackQ *m_CurrSendQ;
        ChannPackQ *m_NextSendQ;

    public:
        // only asio main loop calls the constructor
        // in NetDriver::ChannBuild() called by std::make_shared<Channel>()
        Channel(uint32_t, asio::ip::tcp::socket);

    public:
        // only asio main loop calls the destructor
        // when one channel eventually get released, recycle the ChannID
        virtual ~Channel();

    public:
        uint32_t ID() const
        {
            return m_ID;
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

    public:
        // family of post facilities, called by one server thread
        // Channel will do compression if needed based on message header code

        // post with a r-ref callback, this is the base of all post function
        // current implementation is based on double-queue method
        // one queue (Q1) is used for store new packages in parallel
        // the other (Q2) queue is used to post all packages in ASIO main loop
        //
        // then Q1 needs to be protected from data race
        // but for Q2 since it's only used in ASIO main loop, we don't need to protect it
        //
        // idea from: https://stackoverflow.com/questions/4029448/thread-safety-for-stl-queue
        bool Post(uint8_t nHC, const uint8_t *pData, size_t nDataLen, std::function<void()> &&fnDone);

    public:
        // post with a const-ref callback
        bool Post(uint8_t nHC, const uint8_t *pData, size_t nDataLen, const std::function<void()> &fnDone)
        {
            return Post(nHC, pData, nDataLen, std::function<void()>(fnDone));
        }

        // post without a callback
        bool Post(uint8_t nHC, const uint8_t *pData, size_t nDataLen)
        {
            return Post(nHC, pData, nDataLen, std::function<void()>());
        }

        // post a message header code without a body
        bool Post(uint8_t nHC, std::function<void()> &&fnDone)
        {
            return Post(nHC, nullptr, 0, std::move(fnDone));
        }

        // post a message header without callback
        bool Post(uint8_t nHC)
        {
            return Post(nHC, std::function<void()>());
        }

        // post a message header code without a body
        bool Post(uint8_t nHC, const std::function<void()> &fnDone)
        {
            return Post(nHC, nullptr, 0, std::function<void()>(fnDone));
        }

        // post a strcutre
        template<typename T> bool Post(uint8_t nHC, const T &stMsgT, std::function<void()> &&fnDone)
        {
            return Post(nHC, (const uint8_t *)(&stMsgT), sizeof(stMsgT), std::move(fnDone));
        }

        // post a strcutre
        template<typename T> bool Post(uint8_t nHC, const T &stMsgT, const std::function<void()> &fnDone)
        {
            return Post(nHC, (const uint8_t *)(&stMsgT), sizeof(stMsgT), fnDone);
        }

        // post a strcutre
        template<typename T> bool Post(uint8_t nHC, const T &stMsgT)
        {
            return Post(nHC, (const uint8_t *)(&stMsgT), sizeof(stMsgT));
        }

    private:
        uint8_t *GetReadBuf(size_t nBufLen)
        {
            m_ReadBuf.resize(nBufLen + 16);
            return &(m_ReadBuf[0]);
        }

        uint8_t *GetDecodeBuf(size_t nBufLen)
        {
            m_DecodeBuf.resize(nBufLen + 16);
            return &(m_DecodeBuf[0]);
        }

    private:
        // functions called by asio main loop only
        // following DoXXXFunc should only be invoked in asio main loop thread
        void DoReadPackHC();
        void DoReadPackBody(size_t, size_t);

        void DoSendPack();

    private:
        // called by asio main loop only
        // only called in Channel::DoReadHC()/DoReadBody()
        bool ForwardActorMessage(uint8_t, const uint8_t *, size_t);

    private:
        // called by one server thread
        // only called in Channel::Post(server_message)
        bool FlushSendQ();

    public:
        // called by asio main loop thread and server thread
        // it atomically set the channel state, which would disable everything
        void Shutdown(bool);

    public:
        bool Launch(const Theron::Address &rstAddr);

    public:
        void BindActor(const Theron::Address &rstAddr)
        {
            // force messages forward to the new actor
            // use post rather than directly assignement
            // since asio main loop thread will access m_BindAddress

            // potential bug:
            // internal actor address won't get update immediately after this call

            auto fnBind = [rstAddr, this]()
            {
                m_BindAddress = rstAddr;
            };
            m_Socket.get_io_service().post(fnBind);
        }
};
