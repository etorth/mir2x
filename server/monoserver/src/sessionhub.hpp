/*
 * =====================================================================================
 *
 *       Filename: sessionhub.hpp
 *        Created: 08/14/2015 11:34:33
 *  Last Modified: 05/24/2016 17:25:53
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
 *                 I made an internal array of session pointer then this helps I use
 *                 SessionID always instead of session pointer outside, it's dangerous
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
#include <functional>
#include <unordered_map>

#include "session.hpp"
#include "cachequeue.hpp"
#include "syncdriver.hpp"
#include "messagepack.hpp"

class Session;
using SessionQ = CacheQueue<Session *, 8192>;
class SessionHub: public SyncDriver
{
    private:
        // facilities for asio
        asio::io_service        m_IO;
        int                     m_Port;
        asio::ip::tcp::endpoint m_EndPoint;
        asio::ip::tcp::acceptor m_Acceptor;
        asio::ip::tcp::socket   m_Socket;

    private:
        SessionQ     m_ValidQ;
        uint32_t     m_Count;
        std::thread *m_Thread;

    private:
        Theron::Address         m_SCAddress;
        std::vector<Session *>  m_SessionV;

    public:
        SessionHub(int, const Theron::Address &);
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

        template<typename... Args>
        void Send(uint32_t nSessionID, Args&&... args)
        {
            // 1. find the corresponding session
        }

    private:
        uint32_t ValidID()
        {
            uint32_t nID = m_Count + 1;
            while(true){
                if(!m_SessionV[nID]){ m_Count = nID; return nID; }

                nID++;
                if(nID == m_SessionV.size()){ nID = 1; }
                if(nID == m_Count){ return 0;}
            }
            return 0;
        }

    public:
        void Bind(const Theron::Address &rstAddr)
        {
            m_SCAddress = rstAddr;
        }

    public:
        Session *Validate(uint32_t nSessionID)
        {
            if(nSessionID > 0 && nSessionID < m_SessionV.size()){
                return m_SessionV[nSessionID];
            }

            return nullptr;
        }

        template<typename... Args> bool Send(uint32_t nSessionID, Args&&... args)
        {
            // it's a broadcast
            if(!nSessionID){
                for(auto pSession: m_SessionV){
                    if(pSession){
                        pSession->Send(std::forward<Args>(args)...);
                    }
                }
                // done
                return true;
            }

            auto *pSession = Validate(nSessionID);
            // didn't find it
            if(!pSession){ return false; }

            // ok we find it
            pSession->Send(std::forward<Args>(args)...);
            return true;
        }

    private:
        void Accept();
};
