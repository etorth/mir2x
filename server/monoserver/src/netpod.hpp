/*
 * =====================================================================================
 *
 *       Filename: netpod.hpp
 *        Created: 08/14/2015 11:34:33
 *  Last Modified: 10/03/2017 22:26:31
 *
 *    Description: this will serve as a stand-alone plugin for monoserver, it creates
 *                 with general info. and nothing will be done till Launch()
 *
 *                 when Launch(Theron::Address) with the actor address of service core
 *                 this pod will start a new thread and run asio::run() inside
 *
 *                 when a new connection request received, if netpod decide to accept
 *                 it, the pod will:
 *
 *                      1. allocate a valid session id and its session instance
 *                      2. inform the service core with the connection session id
 *
 *                 then all service core <-> pod communication for this connection will
 *                 based on the session id.
 *
 *                 NetPodN doesn't need a Bind() method since it only communicates with
 *                 ServiceCore, and this address is provided with Launch(), the member
 *                 function Bind() used to forward to Session
 *
 *                 I have three design for this class, but I decide to make it global
 *                 reason as following:
 *
 *                 1. put it in class MonoServer
 *                    the best is to put net pod as a private class in class monoserver
 *                    but netpod is a complex class, I decide only put those const, log
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
 *                    class, however netpodn need the address of servicecore before
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

#include <thread>
#include <cstdint>
#include <asio.hpp>
#include <Theron/Theron.h>

#include "session.hpp"
#include "sysconst.hpp"
#include "monoserver.hpp"
#include "cachequeue.hpp"
#include "syncdriver.hpp"
#include "messagepack.hpp"

class NetPodN: public SyncDriver
{
    private:
        unsigned int                m_Port;
        asio::io_service           *m_IO;
        asio::ip::tcp::endpoint    *m_EndPoint;
        asio::ip::tcp::acceptor    *m_Acceptor;
        asio::ip::tcp::socket      *m_Socket;

    private:
        std::thread                            m_Thread;
        CacheQueue<uint32_t, SYS_MAXPLAYERNUM> m_ValidQ;
        std::mutex                             m_LockV      [SYS_MAXPLAYERNUM];
        Session                               *m_SessionV[2][SYS_MAXPLAYERNUM];

    private:
        Theron::Address m_SCAddress;

    public:
        NetPodN();
       ~NetPodN();

    protected:
        Session *Validate(uint32_t, bool);

    protected:
        bool CheckPort(uint32_t);
        bool InitASIO(uint32_t);

    public:
        // before call this function, the service core should be already
        // after it connection request will be accepted and forward to the core
        //
        // return value:
        //      0: OK
        //      1: invalid argument
        //      2: asio initialization failed
        int Launch(uint32_t, const Theron::Address &);

        // start the specified session with specified actor address
        // return value
        //      0: OK
        //      1: invalid argument
        //      2: there is no half-done session in the slot
        //      3: this session is running
        //      4: session launch error
        int Activate(uint32_t, const Theron::Address &);

    public:
        bool Shutdown(uint32_t nSessionID = 0)
        {
            if(!nSessionID){
                for(uint32_t nSID = 1; nSID < SYS_MAXPLAYERNUM; ++nSID){
                    if(m_SessionV[0][nSID]){
                        delete m_SessionV[0][nSID];
                        m_SessionV[0][nSID] = nullptr;
                    }

                    if(m_SessionV[1][nSID]){
                        m_SessionV[1][nSID]->Shutdown();
                        delete m_SessionV[1][nSID];
                        m_SessionV[1][nSID] = nullptr;
                    }
                }

                // stop the net IO
                m_IO->stop();
                if(m_Thread.joinable()){ m_Thread.join(); }
                return true;
            }

            if(nSessionID < SYS_MAXPLAYERNUM){
                m_SessionV[1][nSessionID]->Shutdown();
                delete m_SessionV[1][nSessionID];
                m_SessionV[1][nSessionID] = nullptr;
                m_ValidQ.PushHead(nSessionID);
                return true;
            }

            return false;
        }

        bool Bind(uint32_t nSessionID, const Theron::Address &rstBindAddr)
        {
            if(true
                    && nSessionID > 0
                    && nSessionID < SYS_MAXPLAYERNUM
                    && m_SessionV[1][nSessionID]){
                m_SessionV[1][nSessionID]->Bind(rstBindAddr);
                return true;
            }
            return false;
        }

    public:
        template<typename... Args> bool Send(uint32_t nSessionID, Args&&... args)
        {
            // it's a broadcast
            // 1. should check the caller's permission
            // 2. how to hanle failure for some sessions when send

            // when some session failed
            // should we send cancel message or leave it as it is?

            if(!nSessionID){
                for(int nSID = 1; nSID < SYS_MAXPLAYERNUM; ++nSID){
                    if(m_SessionV[1][nSID]){
                        if(!m_SessionV[1][nSID]->Send(std::forward<Args>(args)...)){
                            return false;
                        }
                    }
                }
                return true;
            }

            if(nSessionID < SYS_MAXPLAYERNUM && m_SessionV[1][nSessionID]){
                return m_SessionV[1][nSessionID]->Send(std::forward<Args>(args)...);
            }

            // something happens...
            return false;
        }

    private:
        void Accept();
};
