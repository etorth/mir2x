/*
 * =====================================================================================
 *
 *       Filename: sessionio.hpp
 *        Created: 08/14/2015 11:34:33
 *  Last Modified: 02/29/2016 02:20:30
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

#include <asio.hpp>
#include <functional>
#include <thread>
#include <unordered_map>

class Session;
class SessionIO final
{
    public:
        SessionIO(int, std::function<void(uint8_t, Session *)>);
       ~SessionIO();

    public:
        void Launch();
        void Stop();
        void Kill(int);

    public:
        void Dispatch(uint8_t, const uint8_t *, size_t);
        void Send(int, uint8_t, const uint8_t *, size_t);

    private:
        void Accept();

    public:
        size_t SessionCount()
        {
            return m_SessionHub.size();
        }

    private:
        // for asio
        asio::io_service        m_IO;
        int                     m_Port;
        asio::ip::tcp::endpoint m_EndPoint;
        asio::ip::tcp::acceptor m_Acceptor;
        asio::ip::tcp::socket   m_Socket;

    private:
        std::thread *m_Thread;
        int          m_MaxID;

    private:
        std::unordered_map<int, Session *>      m_SessionHub;
        std::function<void(uint8_t, Session *)> m_OperateFunc;
};
