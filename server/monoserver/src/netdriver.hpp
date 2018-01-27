/*
 * =====================================================================================
 *
 *       Filename: netdriver.hpp
 *        Created: 08/14/2015 11:34:33
 *    Description:
 *
 *                 Stand-alone network module for monoserver, this module only do
 *                 read/write to net IO and never pass the package
 *
 *                 when Launch(Theron::Address) with the actor address of service core
 *                 this pod will start a new thread and run asio::run() inside
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

#include <atomic>
#include <thread>
#include <cstdint>
#include <asio.hpp>
#include <Theron/Theron.h>

#include "channel.hpp"
#include "sysconst.hpp"
#include "monoserver.hpp"
#include "cachequeue.hpp"
#include "dispatcher.hpp"
#include "messagepack.hpp"

class NetDriver: public Dispatcher
{
    private:
        unsigned int                m_Port;
        asio::io_service           *m_IO;
        asio::ip::tcp::endpoint    *m_EndPoint;
        asio::ip::tcp::acceptor    *m_Acceptor;
        asio::ip::tcp::socket      *m_Socket;

    private:
        std::thread m_Thread;

    private:
        Theron::Address m_SCAddress;

    private:
        Channel m_ChannelList[SYS_MAXPLAYERNUM];

    private:
        CacheQueue<uint32_t, SYS_MAXPLAYERNUM> m_ValidQ;

    public:
        NetDriver();

    public:
        virtual ~NetDriver();

    protected:
        bool CheckPort(uint32_t);
        bool InitASIO(uint32_t);

    public:
        // launch the net driver with (port, service_core_address)
        // before call this function, the service core should be ready
        // then connection request will be accepted and forward to the service core
        //
        // return value:
        //      0: OK
        //      1: invalid argument
        //      2: asio initialization failed
        int Launch(uint32_t, const Theron::Address &);

    private:
        std::shared_ptr<Channel> ChannBuild(asio::ip::tcp::socket stSocket)
        {
            return std::make_shared<Channel>(std::move(stSocket));
        }

        void ChannRelease()
        {

        }

    private:
        void AcceptNewConnection();
};
