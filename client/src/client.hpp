/*
 * =====================================================================================
 *
 *       Filename: client.hpp
 *        Created: 08/12/2015 09:59:15
 *    Description: public API for class client only
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
#include <SDL2/SDL.h>

#include "netio.hpp"
#include "process.hpp"
#include "message.hpp"
#include "sdldevice.hpp"
#include "raiitimer.hpp"
#include "cachequeue.hpp"

class ProcessRun;
class Client final
{
    private:
        struct SMProcMonitor
        {
            uint64_t ProcTick;
            uint32_t RecvCount;

            SMProcMonitor()
                : ProcTick(0)
                , RecvCount(0)
            {}
        };

        struct ClientMonitor
        {
            std::array<SMProcMonitor, SM_MAX> SMProcMonitorList;
            ClientMonitor()
                : SMProcMonitorList()
            {}
        };

    private:
        ClientMonitor m_clientMonitor;

    private:
        hres_timer m_clientTimer;

    private:
        double m_serverDelay;
        double m_netPackTick;

    private:
        NetIO m_netIO;

    private:
        int m_requestProcess;
        Process *m_currentProcess;

    private:
        std::string m_clipboardBuf;

    public:
        Client();
       ~Client();

    public:
        void mainLoop();

    public:
        void Clipboard(const std::string &szInfo)
        {
            m_clipboardBuf = szInfo;
        }

        std::string Clipboard()
        {
            return m_clipboardBuf;
        }

    public:
        int RequestProcess() const
        {
            return m_requestProcess;
        }

        void RequestProcess(int nProcessID)
        {
            m_requestProcess = nProcessID;
        }

    private:
        void SwitchProcess();
        void SwitchProcess(int);
        void SwitchProcess(int, int);

    public:
        void initASIO();

    private:
        void onServerMessage(uint8_t, const uint8_t *, size_t);

    private:
        void EventDelay(double);
        void processEvent();

    private:
        void draw()
        {
            if(m_currentProcess){
                m_currentProcess->draw();
            }
        }

        void update(double fUpdateTime)
        {
            if(m_currentProcess){
                m_currentProcess->update(fUpdateTime);
            }
        }

    public:
        Process *ProcessValid(int nProcessID)
        {
            return (m_currentProcess && m_currentProcess->ID() == nProcessID) ? m_currentProcess : nullptr;
        }

    public:
        ProcessRun *processRun()
        {
            return ProcessValid(PROCESSID_RUN) ? (ProcessRun *)(m_currentProcess) : nullptr;
        }

        ProcessRun *processRunEx()
        {
            if(auto p = processRun()){
                return p;
            }
            throw fflerror("not in process run");
        }

    private:
        void sendCMsgLog(uint8_t);
        void sendSMsgLog(uint8_t);

    public:
        template<typename... U> void send(uint8_t headCode, U&&... u)
        {
            m_netIO.send(headCode, std::forward<U>(u)...);
            sendCMsgLog(headCode);
        }

    public:
        void PrintMonitor() const;
};
