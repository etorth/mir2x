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
#include <asio.hpp>
#include <array>
#include <mutex>
#include <memory>
#include <vector>
#include <atomic>
#include <cstdint>
#include <functional>
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
        std::atomic<int> m_state;

    private:
        Dispatcher m_dispatcher;

    private:
        asio::ip::tcp::socket m_socket;
        const std::string     m_IP;
        const uint32_t        m_port;

    private:
        // for read channel packets
        // only asio main loop accesses these fields
        uint8_t  m_readHC;
        uint8_t  m_readLen[4];
        uint32_t m_bodyLen;

    private:
        std::vector<uint8_t> m_readBuf;
        std::vector<uint8_t> m_decodeBuf;

    private:
        uint64_t m_bindUID;

    private:
        // for post channel packets
        // server thread and asio main loop accesses these feilds
        //
        // 1. m_flushFlag indicates there is procedure accessing m_currSendQ in asio main loop
        //    m_flushFlag prevents more than one procedure from accessing m_currSendQ
        //
        // 2. m_nextQLock protect the pending queue: m_nextSendQ
        //    m_nextSendQ will be accessed by server thread to push packets in
        bool       m_flushFlag;
        std::mutex m_nextQLock;

    private:
        ChannPackQ  m_sendPackQ0;
        ChannPackQ  m_sendPackQ1;
        ChannPackQ *m_currSendQ;
        ChannPackQ *m_nextSendQ;

    public:
        // only asio main loop calls the constructor
        // in NetDriver::ChannBuild() called by std::make_shared<Channel>()
        Channel(uint32_t, asio::ip::tcp::socket);

    public:
        // only asio main loop calls the destructor
        // when one channel eventually get released, recycle the channID
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
            return m_port;
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
            m_readBuf.resize(nBufLen + 16);
            return &(m_readBuf[0]);
        }

        uint8_t *GetDecodeBuf(size_t nBufLen)
        {
            m_decodeBuf.resize(nBufLen + 16);
            return &(m_decodeBuf[0]);
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
        bool forwardActorMessage(uint8_t, const uint8_t *, size_t);

    private:
        // called by one server thread
        // only called in Channel::Post(server_message)
        bool FlushSendQ();

    public:
        // called by asio main loop thread and server thread
        // it atomically set the channel state, which would disable everything
        void Shutdown(bool);

    public:
        bool Launch(uint64_t);

    public:
        void BindActor(uint64_t nUID)
        {
            // force messages forward to the new actor
            // use post rather than directly assignement
            // since asio main loop thread will access m_bindUID

            // potential bug:
            // internal actor address won't get update immediately after this call

            auto fnBind = [nUID, this]()
            {
                m_bindUID = nUID;
            };
            m_socket.get_io_service().post(fnBind);
        }
};
