/*
 * =====================================================================================
 *
 *       Filename: netio.hpp
 *        Created: 09/03/2015 03:49:00 AM
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
#include <asio.hpp>
#include <queue>
#include <functional>
#include "memorychunkpn.hpp"

class NetIO final
{
    private:
        struct SendPack
        {
            uint8_t headCode;
            const uint8_t *data;        // buffer from internal pool
            size_t         dataLen;     // data size rather the buffer capacity
            SendPack(uint8_t, const uint8_t *, size_t);
        };

    private:
        asio::io_service        m_io;
        asio::ip::tcp::resolver m_resolver;
        asio::ip::tcp::socket   m_socket;

    private:
        uint8_t              m_ReadHC;
        uint8_t              m_ReadLen[4];
        std::vector<uint8_t> m_ReadBuf;

    private:
        std::function<void(uint8_t, const uint8_t *, size_t)> m_msgHandler;

    private:
        std::queue<SendPack> m_sendQueue;

    private:
        MemoryChunkPN<64, 256, 1> m_MemoryPN;

    public:
        NetIO();

    public:
        ~NetIO();

    public:
        void start(const char *, const char *, const std::function<void(uint8_t, const uint8_t *, size_t)> &);

    public:
        void poll()
        {
            m_io.poll();
        }

        void stop()
        {
            m_io.post([this]()
            {
                shutdown();
            });
        }

    public:
        // basic function for the send function family
        // caller will provide the send buffer but NetIO will make an internal copy
        bool send(uint8_t, const uint8_t *, size_t);

    public:
        bool send(uint8_t headCode)
        {
            return send(headCode, nullptr, 0);
        }

        template<typename T> bool send(uint8_t headCode, const T &stMsg)
        {
            static_assert(std::is_trivially_copyable<T>::value, "POD type required");
            return send(headCode, (const uint8_t *)(&stMsg), sizeof(stMsg));
        }

    private:
        void sendHeadCode();
        void sendBuf();
        void sendNext();

    private:
        void readHeadCode();
        bool readBody(size_t, size_t);

    private:
        void shutdown()
        {
            m_io.stop();
        }
};
