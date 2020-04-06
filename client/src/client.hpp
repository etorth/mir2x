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
        ClientMonitor m_ClientMonitor;

    private:
        hres_timer m_ClientTimer;

    private:
        double m_ServerDelay;
        double m_NetPackTick;

    private:
        NetIO m_NetIO;

    private:
        int m_RequestProcess;
        Process *m_CurrentProcess;

    private:
        std::string m_ClipboardBuf;

    public:
        Client();
       ~Client();

    public:
        void MainLoop();

    public:
        void Clipboard(const std::string &szInfo)
        {
            m_ClipboardBuf = szInfo;
        }

        std::string Clipboard()
        {
            return m_ClipboardBuf;
        }

    public:
        int RequestProcess() const
        {
            return m_RequestProcess;
        }

        void RequestProcess(int nProcessID)
        {
            m_RequestProcess = nProcessID;
        }

    private:
        void SwitchProcess();
        void SwitchProcess(int);
        void SwitchProcess(int, int);

    public:
        void InitASIO();
        void PollASIO();
        void StopASIO();

    private:
        void OnServerMessage(uint8_t, const uint8_t *, size_t);

    private:
        void EventDelay(double);
        void processEvent();

    private:
        void Draw()
        {
            if(m_CurrentProcess){
                m_CurrentProcess->Draw();
            }
        }

        void Update(double fDTime)
        {
            if(m_CurrentProcess){
                m_CurrentProcess->Update(fDTime);
            }
        }

    public:
        Process *ProcessValid(int nProcessID)
        {
            return (m_CurrentProcess && m_CurrentProcess->ID() == nProcessID) ? m_CurrentProcess : nullptr;
        }

    public:
        template<typename... U> void Send(U&&... u)
        {
            m_NetIO.Send(std::forward<U>(u)...);
        }

        void sendNPCEventId(uint64_t)
        {
        }

    public:
        void PrintMonitor() const;
};
