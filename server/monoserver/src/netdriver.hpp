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

#include <asio.hpp>
#include <atomic>
#include <thread>
#include <cstdint>
#include "channel.hpp"
#include "sysconst.hpp"
#include "monoserver.hpp"
#include "cachequeue.hpp"
#include "dispatcher.hpp"
#include "messagepack.hpp"

class Channel;
class NetDriver final: public Dispatcher
{
    private:
        friend class Channel;

    private:
        unsigned int                m_Port;
        asio::io_service           *m_IO;
        asio::ip::tcp::endpoint    *m_EndPoint;
        asio::ip::tcp::acceptor    *m_Acceptor;
        asio::ip::tcp::socket      *m_Socket;

    private:
        std::thread m_Thread;

    private:
        uint64_t m_ServiceCoreUID;

    private:
        CacheQueue<uint32_t, SYS_MAXPLAYERNUM> m_ChannIDQ;

    private:
        std::shared_ptr<Channel> m_ChannelList[SYS_MAXPLAYERNUM + 1];

    public:
        NetDriver();

    public:
        virtual ~NetDriver();

    protected:
        bool CheckPort(uint32_t);
        bool InitASIO(uint32_t);

    private:
        void RecycleChannID(uint32_t nChannID)
        {
            m_ChannIDQ.PushBack(nChannID);
        }

    private:
        bool CheckChannID(uint32_t nChannID)
        {
            return (nChannID > 0) && (nChannID <= std::extent<decltype(m_ChannelList)>::value);
        }

    public:
        // launch the net driver with (port, service_core_address)
        // before call this function, the service core should be ready
        // then connection request will be accepted and forward to the service core
        //
        // return value:
        //      0: OK
        //      1: invalid argument
        //      2: asio initialization failed
        bool Launch(uint32_t, uint64_t);

    public:
        template<typename... Args> bool Post(uint32_t nChannID, uint8_t nHC, Args&&... args)
        {
            if(CheckChannID(nChannID)){
                return m_ChannelList[nChannID]->Post(nHC, std::forward<Args>(args)...);
            }
            return false;
        }

        void BindActor(uint32_t nChannID, uint64_t nUID)
        {
            if(CheckChannID(nChannID)){
                m_ChannelList[nChannID]->BindActor(nUID);
            }
        }

        void Shutdown(uint32_t nChannID, bool bForce)
        {
            if(CheckChannID(nChannID)){
                m_ChannelList[nChannID]->Shutdown(bForce);
            }
        }

    private:
        bool ChannBuild(uint32_t nChannID, asio::ip::tcp::socket stSocket)
        {
            if(CheckChannID(nChannID)){
                m_ChannelList[nChannID] = std::make_shared<Channel>(nChannID, std::move(stSocket));
                return true;
            }
            return false;
        }

        void ChannRelease(uint32_t nChannID)
        {
            if(CheckChannID(nChannID)){
                m_ChannelList[nChannID].reset();
            }
        }

    private:
        void AcceptNewConnection();
};
