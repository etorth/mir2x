/*
 * =====================================================================================
 *
 *       Filename: sessionhub.hpp
 *        Created: 08/14/2015 11:34:33
 *  Last Modified: 04/23/2016 00:07:42
 *
 *    Description: session hub, to create and destroy all session's. create with
 *                 callback function fnOperateHC, listen port, and service core
 *                 address.
 *
 *                 when launched, sessioin hub will listen on the port, if accepted
 *                 any connection request, forward it to service core for comfirm
 *                 or refuse.
 *
 *                 if refused, delete the temporary session, otherwise wait further
 *                 operation of the session. when get all needed information, pass
 *                 the session pointer to service, it will forward eventually to a
 *                 player object. when the life cycle of the player object ends,
 *                 this pointer will be pass back and destroyed (with its record in
 *                 the map) by session hub.
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
#include <Theron/Theron.h>

#include <thread>
#include <cstdint>
#include <functional>
#include <unordered_map>

#include "log.hpp"

class Session;
class SessionHub
{
    private:
        // facilities for asio
        asio::io_service        m_IO;
        int                     m_Port;
        asio::ip::tcp::endpoint m_EndPoint;
        asio::ip::tcp::acceptor m_Acceptor;
        asio::ip::tcp::socket   m_Socket;

    private:
        uint32_t     m_MaxID;
        std::thread *m_Thread;

    private:
        // facilities for communication with service core
        Theron::Address              m_ServiceCoreAddress;
        Theron::Receiver             m_Receiver;
        Theron::Catcher<MessagePack> m_Catcher;

    private:
        std::function<void(uint8_t, Session *)> m_OperateFunc;
        std::unordered_map<uint32_t, Session *> m_SessionMap;

    public:
        SessionHub(int, const Theron::Address &, const std::function<void(uint8_t, Session *)> &);
        virtual ~SessionHub();

    public:
        // before call this function, the service core should be already
        // after it connection request will be accepted and forward to the core
        void Launch();

        // we don't have ``suspend", shutdown means totally disable it
        // and erase the record from the hub
        void Shutdown(uint32_t);

        void Shutdown()
        {
            Shutdown(0);
        }

    public:
        // send by SID, rarely used actually
        void Send(uint32_t, uint8_t, const uint8_t *, size_t);

    public:
        Session *Validate(int nSID)
        {
            auto pRecord = m_SessionMap.find(nSID);
            if(pRecord != m_SessionMap.end()){
                return pRecord->second;
            }
            return nullptr;
        }

    public:
        size_t SessionCount()
        {
            return m_SessionMap.size();
        }

    private:
        void Accept();
};
