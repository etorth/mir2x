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
            uint64_t procTick  = 0;
            uint32_t recvCount = 0;
        };

        struct ClientMonitor
        {
            std::array<SMProcMonitor, SM_END> SMProcMonitorList;
            ClientMonitor()
                : SMProcMonitorList()
            {}
        };

    private:
        std::string m_token;

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
        int m_requestProcess = PROCESSID_NONE;
        std::unique_ptr<Process> m_currentProcess;

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
        void requestProcess(int processID)
        {
            m_requestProcess = processID;
        }

    private:
        void switchProcess();
        void switchProcess(int);
        void switchProcess(int, int);

    public:
        void initASIO();

    private:
        void onServerMessage(uint8_t, const uint8_t *, size_t);

    private:
        void eventDelay(double);
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
        Process *ProcessValid(int processID)
        {
            return (m_currentProcess && m_currentProcess->id() == processID) ? m_currentProcess.get() : nullptr;
        }

    public:
        ProcessRun *processRun()
        {
            return ProcessValid(PROCESSID_RUN) ? (ProcessRun *)(m_currentProcess.get()) : nullptr;
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

    public:
        void setToken(std::string token)
        {
            m_token = std::move(token);
        }

        const std::string &getToken() const
        {
            return m_token;
        }
};
