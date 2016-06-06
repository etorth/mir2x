/*
 * =====================================================================================
 *
 *       Filename: netpod.hpp
 *        Created: 08/14/2015 11:34:33
 *  Last Modified: 06/05/2016 22:34:36
 *
 *    Description: this will serve as a stand-alone plugin for monoserver, it creates
 *                 with general info. and nothing will be done till Launch()
 *
 *                 when Launch(Theron::Address) with an actor address, this pod will
 *                 start a new thrad and run asio::run() with it, all accepted conn-
 *                 ection request will be send to this actor address
 *
 *                 if the connection request is accpeted, then the pod create a
 *                 session with NetID inside, all communication will through this ID
 *
 *                 NetPod doesn't need a Bind() method since it only communicates with
 *                 ServiceCore, only this address, we provide with Launch()
 *
 *                 TODO & TBD
 *                 make it global, I think it for a long time. reason:
 *
 *                 1. put it in class MonoServer
 *                    I was thinking of only put those const, log and tips in MonoServer,
 *                    and obviously NetPod is a complex class.
 *
 *                 2. put it in ServiceCore
 *                    then it can use SyncDirver <-> Actor channel. But this require that
 *                    ServiceCore need to forward pointer of Session to class Player, and
 *                    validate of the pointer is complex. what's more, if an actor want
 *                    to send a net message, it need to send a actor message to SC first.
 *
 *                 3. put it as a global variable
 *                    drawbacks: I have a rule that a global class is a self-contained
 *                    class, however NetPod need the address of ServiceCore before
 *                    activated. currently I have to make it as a parameter for Launch()
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
#include "monoserver.hpp"
#include "cachequeue.hpp"
#include "syncdriver.hpp"
#include "messagepack.hpp"

template<size_t PodSize> class NetPod: public SyncDriver
{
    private:
        // facilities for asio
        unsigned int                m_Port;
        asio::io_service           *m_IO;
        asio::ip::tcp::endpoint    *m_EndPoint;
        asio::ip::tcp::acceptor    *m_Acceptor;
        asio::ip::tcp::socket      *m_Socket;

        // for session slot
        std::thread                 m_Thread;
        CacheQueue<size_t, PodSize> m_ValidQ;
        Session                    *m_SessionV[2][PodSize];
        std::mutex                  m_LockV[PodSize];

        // for service core address
        Theron::Address             m_SCAddress;

    public:
        NetPod()
            : SyncDriver()
            , m_Port(0)
            , m_IO(nullptr)
            , m_EndPoint(nullptr)
            , m_Acceptor(nullptr)
            , m_Socket(nullptr)
            , m_Thread()
            , m_ValidQ()
            , m_SCAddress(Theron::Address::Null())
        {
            for(size_t nSID = 0; nSID < PodSize; ++nSID){
                m_SessionV[0][nSID] = nullptr;
                m_SessionV[1][nSID] = nullptr;
            }
        }

        virtual ~NetPod()
        {
            Shutdown(0);
            if(m_Thread.joinable()){ m_Thread.join(); }

            delete m_Socket;
            delete m_Acceptor;
            delete m_EndPoint;
            delete m_IO;
        }

    protected:
        // TODO & TBD: assume ASIO has been ready
        Session *Validate(uint32_t nSessionID, bool bValid)
        {
            if(nSessionID > 0 && nSessionID < PodSize){
                return m_SessionV[bValid ? 1 : 0][nSessionID];
            }

            return nullptr;
        }

    protected:
        bool CheckPort(uint32_t nPort)
        {
            if(nPort <= 1024){
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Don't use reserved port");
                return false;
            }

            if(m_Port > 1024){
                // TODO here we add other well-unknown occupied port check
            }

            return true;
        }

        // TODO stop io before we restart it
        bool InitASIO(uint32_t nPort)
        {
            // 1. set server listen port

            if(!CheckPort(nPort)){
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid port provided");
                return false;
            }

            m_Port = nPort;

            try{
                m_IO       = new asio::io_service();
                m_EndPoint = new asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_Port);
                m_Acceptor = new asio::ip::tcp::acceptor(*m_IO, *m_EndPoint);
                m_Socket   = new asio::ip::tcp::socket(*m_IO);
            }catch(...){
                delete m_Socket;
                delete m_Acceptor;
                delete m_EndPoint;
                delete m_IO;

                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Initialization of ASIO failed");
                g_MonoServer->Restart();
            }

            return true;
        }

    public:
        // before call this function, the service core should be already
        // after it connection request will be accepted and forward to the core
        //
        // return value:
        //      0: OK
        //      1: invalid argument
        //      2: asio initialization failed
        int Launch(uint32_t nPort, const Theron::Address &rstSCAddr)
        {
            // 1. check parameter
            if(!CheckPort(nPort) || rstSCAddr == Theron::Address::Null()){ return 1; }

            // 2. assign the target address
            m_SCAddress = rstSCAddr;

            // 3. make sure the internal thread has ended
            if(m_Thread.joinable()){ m_Thread.join(); }

            // 4. prepare valid session ID
            m_ValidQ.Clear();
            for(uint32_t nID = 1; nID < PodSize; ++nID){
                m_ValidQ.PushHead(nID);
            }

            // 5. init ASIO
            if(!InitASIO(nPort)){ return 2; }

            // 6. put one accept handler inside the event loop
            Accept();

            // 7. start the internal thread
            m_Thread = std::thread([this](){ m_IO->run(); });

            // 8. all Launch() function will return 0 when succceeds
            return 0;
        }

        // start the specified session with specified actor address
        // return value
        //      0: OK
        //      1: invalid argument
        //      2: there is no half-done session in the slot
        //      3: this session is running
        //      4: session launch error
        int Activate(uint32_t nSessionID, const Theron::Address &rstTargetAddress)
        {
            // 1. check argument
            if(false
                    || nSessionID == 0
                    || nSessionID >= PodSize
                    || rstTargetAddress == Theron::Address::Null()){
                return 1;
            }

            // 2. get pointer
            if(!m_SessionV[0][nSessionID]){ return 2; }

            // 3. corresponding running slot is empty?
            if(m_SessionV[1][nSessionID]){ return 3; }

            // 4. start it and move it to the running slot
            if(m_SessionV[0][nSessionID]->Launch(rstTargetAddress)){ return 4; }

            // 5. launch ok
            std::swap(m_SessionV[0][nSessionID], m_SessionV[1][nSessionID]);
            m_SessionV[0][nSessionID] = nullptr;

            return 0;
        }

        bool Shutdown(uint32_t nSessionID = 0)
        {
            if(!nSessionID){
                for(uint32_t nSID = 1; nSID < PodSize; ++nSID){
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

            if(nSessionID < PodSize){
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
            if(nSessionID > 0 && nSessionID < PodSize && m_SessionV[1][nSessionID]){
                m_SessionV[1][nSessionID]->Bind(rstBindAddr);
                return true;
            }
            return false;
        }


    public:
        template<typename... Args> bool Send(uint32_t nSessionID, Args&&... args)
        {
            // it's a broadcast
            if(!nSessionID){
                for(int nSID = 1; nSID < (int)PodSize; ++nSID){
                    if(m_SessionV[1][nSID]){
                        m_SessionV[1][nSID]->Send(std::forward<Args>(args)...);
                    }
                }
                return true;
            }

            if(nSessionID < PodSize && m_SessionV[1][nSessionID]){
                m_SessionV[1][nSessionID]->Send(std::forward<Args>(args)...);
                return true;
            }

            // something happens...
            return false;
        }

    private:
        void Accept()
        {
            auto fnAccept = [this](std::error_code stEC){
                extern MonoServer *g_MonoServer;
                if(stEC){
                    // error occurs, stop the network
                    // assume g_MonoServer is ready for log
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Network error when accepting");
                    g_MonoServer->Restart();

                    // we won't put Accept() in the event loop again
                    // then the IO will stop after this
                    return;
                }

                g_MonoServer->AddLog(LOGTYPE_INFO, "Connection requested from (%s:%d)",
                        m_Socket->remote_endpoint().address().to_string().c_str(),
                        m_Socket->remote_endpoint().port());

                if(m_ValidQ.Empty()){
                    g_MonoServer->AddLog(LOGTYPE_INFO, "No valid slot for new connection request, refused");
                    return;
                }

                uint32_t nValidID = m_ValidQ.Head();
                m_ValidQ.PopHead();

                if(!nValidID){
                    g_MonoServer->AddLog(LOGTYPE_INFO, "Mystious error, could never happen");
                    return;
                }

                // create the new session and put it in V[0]
                m_SessionV[0][nValidID] = new Session(nValidID, std::move(*m_Socket));

                // inform the serice core that there is a new connection
                if(Forward({MPK_NEWCONNECTION, nValidID}, m_SCAddress)){
                    delete m_SessionV[0][nValidID];
                    m_SessionV[0][nValidID] = nullptr;

                    m_ValidQ.PushHead(nValidID);
                    g_MonoServer->AddLog(LOGTYPE_INFO, "Can't inform ServiceCore a new connection");
                    return;
                }

                g_MonoServer->AddLog(LOGTYPE_INFO, "Informed ServiceCore for a new connection");

                // accept next request
                Accept();
            };

            m_Acceptor->async_accept(*m_Socket, fnAccept);
        }
};

using NetPodN = NetPod<8192>;
