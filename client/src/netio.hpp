/*
 * =====================================================================================
 *
 *       Filename: netio.hpp
 *        Created: 09/03/2015 03:49:00 AM
 *  Last Modified: 04/26/2017 11:55:49
 *
 *    Description: read / write for 1 to 1 map network, for the server part we use class
 *                 session since it's 1 to N map.
 *
 *                 this class provide an internal buffer to store read data, when using
 *                 internal buffer we need to make the read procedure one by one otherwise
 *                 data will be corrupted
 *
 *                 for sending message, I put the memory pool inside to free user to
 *                 mantain the message buffer, and I put compression support inside
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

#include "memorychunkpn.hpp"

class NetIO final
{
    private:
        struct SendPack
        {
            uint8_t  HC;

            const uint8_t *Data;        // buffer from internal pool
            size_t         DataLen;     // data size rather the buffer capacity

            std::function<void()> OnDone;

            // constructor of SendPack
            // we don't define the destructor of SendPack
            // Data will be explicitly released in DoSendNext()
            SendPack(uint8_t, const uint8_t *, size_t, std::function<void()> &&);
        };

    private:
        asio::io_service        m_IO;
        asio::ip::tcp::resolver m_Resolver;
        asio::ip::tcp::socket   m_Socket;

    private:
        uint8_t              m_ReadHC;
        uint8_t              m_ReadLen[4];
        std::vector<uint8_t> m_ReadBuf;

    private:
        // Game::InitASIO() provide the completion handler for read messages
        // the handler will handle a fully received message instead of (HC, Body) seperately
        std::function<void(uint8_t, const uint8_t *, size_t)> m_OnReadDone;

    private:
        std::queue<SendPack> m_SendQueue;

    private:
        MemoryChunkPN<64, 256, 1> m_MemoryPN;

    public:
        NetIO();
       ~NetIO();

    public:
        bool InitIO(const char *, const char *, const std::function<void(uint8_t, const uint8_t *, size_t)> &);
        void PollIO();
        void StopIO();

    private:
        void Shutdown();

    public:
        // basic function for the send function family
        // caller will provide the send buffer but NetIO will make an internal copy
        bool Send(uint8_t, const uint8_t *, size_t, std::function<void()>&&);

    public:
        bool Send(uint8_t nHC)
        {
            return Send(nHC, nullptr, 0, std::function<void()>());
        }

        bool Send(uint8_t nHC, std::function<void()> &&fnDone)
        {
            return Send(nHC, nullptr, 0, std::move(fnDone));
        }

        bool Send(uint8_t nHC, const std::function<void()> &fnDone)
        {
            return Send(nHC, nullptr, 0, std::function<void()>(fnDone));
        }

        bool Send(uint8_t nHC, const uint8_t *pBuf, size_t nBufLen)
        {
            return Send(nHC, pBuf, nBufLen, std::function<void()>());
        }

        bool Send(uint8_t nHC, const uint8_t *pBuf, size_t nBufLen, const std::function<void()>& fnDone)
        {
            return Send(nHC, pBuf, nBufLen, std::function<void()>(fnDone));
        }

        template<typename T> bool Send(uint8_t nHC, const T &stMsg, std::function<void()> &&fnDone)
        {
            static_assert(std::is_pod<T>::value, "pod type required");
            return Send(nHC, (const uint8_t *)(&stMsg), sizeof(stMsg), std::move(fnDone));
        }

        template<typename T> bool Send(uint8_t nHC, const T &stMsg, const std::function<void()> &fnDone)
        {
            return Send<T>(nHC, stMsg, std::function<void()>(fnDone));
        }

        template<typename T> bool Send(uint8_t nHC, const T &stMsg)
        {
            return Send<T>(nHC, stMsg, std::function<void()>());
        }

    private:
        void DoSendHC();
        void DoSendBuf();
        void DoSendNext();

    private:
        void DoReadHC();
        bool DoReadBody(size_t, size_t);
};
