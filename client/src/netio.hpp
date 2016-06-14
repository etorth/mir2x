/*
 * =====================================================================================
 *
 *       Filename: netio.hpp
 *        Created: 09/03/2015 03:49:00 AM
 *  Last Modified: 06/13/2016 21:41:01
 *
 *    Description: read / write for 1 to 1 map network, for the server part we use class
 *                 session since it's 1 to N map.
 *
 *                 this class provide an internal buffer to store read data, but we can
 *                 provide the buffer by ourself. When using internal buffer we need to
 *                 make the read procedure one by one otherwise data will be corrupted
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
#include <asio.hpp>
#include <functional>
#include <type_traits>

class NetIO final
{
    private:
        // 1. facilities of asio
        asio::io_service        m_IO;
        asio::ip::tcp::resolver m_Resolver;
        asio::ip::tcp::socket   m_Socket;

        // 2. internal message read buffer
        uint8_t                 m_MsgHC;
        std::vector<uint8_t>    m_Buf;

        // 3. send queue, this provides data stream in send order, user should maintain to
        //    keep the sending buffer valid during the pending duration
        //       0. hard code
        //       1. buffer
        //       2. buffer size
        //       3. callback after this message has been sent
        std::queue<std::tuple<uint8_t, const uint8_t *, size_t, std::function<void()>>> m_WQ;

    public:
        NetIO();
        ~NetIO();

    public:
        void RunIO(const char *, const char *, const std::function<void(uint8_t)> &);
        void StopIO();

        // TODO this two function is prepared for synchronized process
        void InitIO(const char *, const char *, const std::function<void(uint8_t)> &);
        void PollIO();


    public:
        // root of the send function family
        //      1. header code
        //      2. buffer pointer
        //      3. buffer length
        //      4. callback
        void Send(uint8_t, const uint8_t *, size_t, std::function<void()>&&);

        // send a head code message with empty content
        void Send(uint8_t nHC)
        {
            Send(nHC, nullptr, 0, std::function<void()>());
        }

        void Send(uint8_t nHC, std::function<void()> &&fnDone)
        {
            Send(nHC, nullptr, 0, std::move(fnDone));
        }

        void Send(uint8_t nHC, const std::function<void()> &fnDone)
        {
            Send(nHC, nullptr, 0, std::function<void()>(fnDone));
        }

        void Send(uint8_t nHC, const uint8_t *pBuf, size_t nBufLen)
        {
            Send(nHC, pBuf, nBufLen, std::function<void()>());
        }

        void Send(uint8_t nHC, const uint8_t *pBuf, size_t nBufLen, const std::function<void()>& fnDone)
        {
            Send(nHC, pBuf, nBufLen, std::function<void()>(fnDone));
        }

        // 1. template T couldn't be copy since NetIO won't maintain the send buffer
        // 2. try to use std::remove_reference_t<T>
        // 3. try to disable user to pass a rvalue here
        template<typename T> void Send(uint8_t nMsgHC, const T &stMsg, std::function<void()> &&fnDone)
        {
            // 1. make sure it's pod type
            static_assert(std::is_pod<T>::value, "pod type required");

            // 2. allcoate a buffer for send
            uint8_t *pBuf = new uint8_t[sizeof(stMsg)];

            // 3. copy to the buffer, then we won't require a static T
            std::memcpy(pBuf, &stMsg, sizeof(stMsg));

            // 4. make a new callback
            auto fnCallback = [pBuf, fnDone = std::move(fnDone)](){
                // 1. call the callback
                if(fnDone){ fnDone(); }

                // 2. free the buffer
                delete [] pBuf;
            };

            // 5. install the handler
            Send(nMsgHC, pBuf, sizeof(stMsg), fnCallback);
        }

        template<typename T> void Send(uint8_t nMsgHC, const T &stMsg, const std::function<void()> &fnDone)
        {
            Send<T>(nMsgHC, stMsg, std::function<void()>(fnDone));
        }

        template<typename T> void Send(uint8_t nMsgHC, const T &stMsg)
        {
            Send<T>(nMsgHC, stMsg, std::function<void()>());
        }

    public:
        void ReadHC(std::function<void(uint8_t)> &&);

        void ReadHC(const std::function<void(uint8_t)> &fnOnHC)
        {
            ReadHC(std::function<void(uint8_t)>(fnOnHC));
        }

        void Read(uint8_t *, size_t, std::function<void(const uint8_t *, size_t)> &&);
        void Read(uint8_t *pBuf, size_t nLen, const std::function<void(const uint8_t *, size_t)> &fnDone)
        {
            Read(pBuf, nLen, std::function<void(const uint8_t *, size_t)>(fnDone));
        }

        // use the internal read buffer, means we have to keep the read request sequentially, and we have
        // to make the handler to accept argument for buffer
        void Read(size_t nLen, std::function<void(const uint8_t *, size_t)> &&fnDone)
        {
            if(m_Buf.size() < nLen){ m_Buf.resize(nLen); }
            Read((uint8_t *)(&(m_Buf[0])), nLen, std::move(fnDone));
        }

        void Read(size_t nLen, const std::function<void(const uint8_t *, size_t)> &fnDone)
        {
            Read(nLen, std::function<void(const uint8_t *, size_t)>(fnDone));
        }

    private:
        void Close();

        void DoSendHC();
        void DoSendBuf();
        void DoSendNext();
};
