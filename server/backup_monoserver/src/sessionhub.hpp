/*
 * =====================================================================================
 *
 *       Filename: sessionhub.hpp
 *        Created: 08/14/2015 11:34:33
 *  Last Modified: 04/18/2016 17:47:03
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
class SessionHub final
{
    public:
        SessionHub(int, std::function<void(uint8_t, Session *)>);
       ~SessionHub();

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
        Session *Validate(int nSessionID)
        {
            auto pInst = m_SessionHub.find(nSessionID);
            if(pInst != m_SessionHub.end()){
                return pInst->second;
            }
            return nullptr;
        }

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
