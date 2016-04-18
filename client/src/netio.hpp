/*
 * =====================================================================================
 *
 *       Filename: netio.hpp
 *        Created: 09/03/2015 03:49:00 AM
 *  Last Modified: 04/17/2016 23:57:13
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
#include <queue>
#include <asio.hpp>
#include <functional>
#include <type_traits>

class NetIO final
{
    private:
        asio::io_service        m_IO;
        asio::ip::tcp::resolver m_Resolver;
        asio::ip::tcp::socket   m_Socket;
        std::vector<uint8_t>    m_Buf;
        uint8_t                 m_MsgHC;
        int                     m_ReadCount;

    private:
        // send queue
        // this provides data stream in send order, user should maintain to keep
        // the sending buffer valid during the pending duration
        // 
        std::queue<std::tuple<
            uint8_t,          // Message HC
            const uint8_t *,  // Message buf, NetIO won't maintain its validation
            size_t>> m_WQ;

    public:
        NetIO();
        ~NetIO();

    public:
        void RunIO(const char *, const char *, const std::function<void(uint8_t)> &);
        void StopIO();

        // TODO
        // this two function is prepare for synchronized process
        void InitIO(const char *, const char *, const std::function<void(uint8_t)> &);
        void PollIO();

    public:
        // send a body-less head code
        void Send(uint8_t);
        // send a HC and its body buffer
        // user should maintain the validation of the buffer
        void Send(uint8_t, const uint8_t *, size_t);

        // 1. template T couldn't be copy since NetIO won't maintain
        //    the send buffer
        // 2. try to use std::remove_reference_t<T>
        // 3. try to disable user to pass a rvalue here
        template<typename T> void Send(uint8_t nMsgHC, T &stMsg)
        {
            Send(nMsgHC, (const uint8_t *)(&stMsg), sizeof(stMsg));
        }

        // read a HC
        // user should provide the method to consume the HC
        void ReadHC(const std::function<void(uint8_t)> &);

        // read specified length of data to the buffer
        // user should maintain the validation of the buffer
        void Read(uint8_t *, size_t, const std::function<void(const uint8_t *, size_t)> &);

        // TODO
        // do I really need it?
        template<typename T> void Read(T &stMsg,
                const std::function<void(const uint8_t *, size_t)> &fnOperateBuf)
        {
            Read((uint8_t *)(&stMsg), sizeof(stMsg), fnOperateBuf);
        }

        // use the internal read buffer, means we have to
        // keep the read request sequentially
        //
        // and we have to make the handler to accept argument for buffer
        void Read(size_t nLen, const std::function<void(const uint8_t *, size_t)> &fnOperateBuf)
        {
            // 1. check the buffer
            if(m_Buf.size() < nLen){ m_Buf.resize(nLen); }
            Read((uint8_t *)(&(m_Buf[0])), nLen, fnOperateBuf);
        }

    private:
        void Close();

        void DoSendHC();
        void DoSendBuf();
        void DoSendNext();
};
