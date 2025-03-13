#pragma once
#include <atomic>
#include <string>
#include <type_traits>
#include <SDL2/SDL.h>

#include "conceptf.hpp"
#include "netio.hpp"
#include "process.hpp"
#include "message.hpp"
#include "sdldevice.hpp"
#include "raiitimer.hpp"

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
        struct ResponseHandler
        {
            uint64_t timeout;
            std::function<void(uint8_t, const uint8_t *, size_t)> handler;
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
        uint64_t m_respHandlerIndex = 1;
        std::unordered_map<uint64_t, ResponseHandler> m_respHandlers;

    private:
        int m_requestProcess = PROCESSID_NONE;
        std::unique_ptr<Process> m_currentProcess;

    private:
        std::optional<SMOnlineOK> m_smOOK;

    private:
        std::string m_clipboardBuf;

    public:
        Client();

    public:
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
                switchProcess();
            }
        }

    public:
        Process *ProcessValid(int processID)
        {
            return (m_currentProcess && m_currentProcess->id() == processID) ? m_currentProcess.get() : nullptr;
        }

    public:
        template<typename T> auto getProcess() const
        {
            return m_currentProcess ? dynamic_cast<T *>(m_currentProcess.get()) : nullptr;
        }

    public:
        void setOnlineOK(const SMOnlineOK &smOOK)
        {
            m_smOOK = smOOK;
        }

    private:
        void sendCMsgLog(uint8_t);
        void sendSMsgLog(uint8_t);

    public:
        void send(const ClientMsgBuf &msg, std::function<void(uint8_t, const uint8_t *, size_t)> fnOp = nullptr)
        {
            if(fnOp){
                if(m_respHandlers.empty()){
                    m_respHandlerIndex = 1;
                }

                m_respHandlers.emplace(m_respHandlerIndex, ResponseHandler{hres_tstamp::localtime() + 1000, std::move(fnOp)});
                m_netIO.send(msg.headCode, msg.data, msg.size, m_respHandlerIndex);
                m_respHandlerIndex++;
            }
            else{
                m_netIO.send(msg.headCode, msg.data, msg.size, 0);
            }

            sendCMsgLog(msg.headCode);
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
