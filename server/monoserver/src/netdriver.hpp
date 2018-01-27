/*
 * =====================================================================================
 *
 *       Filename: netdriver.hpp
 *        Created: 08/14/2015 11:34:33
 *    Description: this will serve as a stand-alone plugin for monoserver, it creates
 *                 with general info. and nothing will be done till Launch()
 *
 *                 when Launch(Theron::Address) with the actor address of service core
 *                 this pod will start a new thread and run asio::run() inside
 *
 *                 when a new connection request received, if net driver decide to accept
 *                 it, the pod will:
 *
 *                      1. allocate a channel, which consists of one session instance
 *                      2. inform the service core with the session id
 *
 *                 then all service core <-> pod communication for this connection will
 *                 based on the session id.
 *
 *                 NetDriver doesn't need a Bind() method since it only communicates with
 *                 ServiceCore, and this address is provided with Launch(), the member
 *                 function Bind() used to forward to Session
 *
 *                 I have three design for this class, but I decide to make it global
 *                 reason as following:
 *
 *                 1. put it in class MonoServer
 *                    the best is to put net pod as a private class in class monoserver
 *                    but net driver is a complex class, I decide only put those const, log
 *                    and tips in monoserver.
 *
 *                 2. put it in ServiceCore
 *                    then it can use syncdirver <-> actor channel. but this require that
 *                    servicecore need to forward pointer of session to class player, and
 *                    validate of the pointer is complex. what's more, if an actor want
 *                    to send a net message, it need to send a actor message to sc first.
 *
 *                 3. put it as a global variable
 *                    drawbacks: i have a rule that a global class is a self-contained
 *                    class, however net driver need the address of servicecore before
 *                    activated. currently I make it as a parameter for Launch()
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

    public:
        // start the specified session with specified actor address
        // 1. before invocation the session should be allcated with proper socket
        // 2. after  invocation the session can do read and write
        //
        // return value
        //      0: OK
        //      1: invalid argument
        //      2: there is no half-done session in the slot
        //      3: this session is running
        //      4: session launch error
        bool Activate(uint32_t, const Theron::Address &);

    public:
        void Shutdown(uint32_t nSessionID = 0)
        {
            switch(nSessionID){
                case 0:
                    {
                        for(int nIndex = 1; nIndex < (int)(std::extent<decltype(m_ChannelList)>::value); ++nIndex){
                            m_ChannelList[nIndex].ChannRelease();
                            m_ValidQ.PushHead((uint32_t)(nIndex));
                        }

                        m_IO->stop();
                        if(m_Thread.joinable()){
                            m_Thread.join();
                        }

                        break;
                    }
                default:
                    {
                        if(nSessionID < (uint32_t)(std::extent<decltype(m_ChannelList)>::value)){
                            m_ChannelList[nSessionID].ChannRelease();
                            m_ValidQ.PushHead(nSessionID);
                        }
                        break;
                    }
            }
        }

        bool Bind(uint32_t nSessionID, const Theron::Address &rstBindAddr)
        {
            if(true
                    && nSessionID
                    && nSessionID < (uint32_t)(std::extent<decltype(m_ChannelList)>::value)){

                m_ChannelList[nSessionID].Bind(rstBindAddr);
                return true;
            }
            return false;
        }

    public:
        template<typename... Args> bool Send(uint32_t nSessionID, uint8_t nHC, Args&&... args)
        {
            // it's a broadcast
            // 1. should check the caller's permission
            // 2. how to hanle failure for some sessions when send

            // when some session failed
            // should we send cancel message or leave it as it is?

            int nIndex0 = -1;
            int nIndex1 = -1;

            switch(nSessionID){
                case 0:
                    {
                        nIndex0 = (int)(1);
                        nIndex1 = (int)(std::extent<decltype(m_ChannelList)>::value);
                        break;
                    }
                default:
                    {
                        if(nSessionID < (uint32_t)(std::extent<decltype(m_ChannelList)>::value)){
                            nIndex0 = (int)(nSessionID + 0);
                            nIndex1 = (int)(nSessionID + 1);
                        }else{
                            return false;
                        }
                        break;
                    }
            }

            for(int nIndex = nIndex0; nIndex < nIndex1; ++nIndex){
                if(!m_ChannelList[nIndex].Send(nHC, std::forward<Args>(args)...)){
                    return false;
                }
            }
            return true;
        }

    private:
        void Accept();
};
