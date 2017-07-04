#pragma once 
#include "sdldevice.hpp"
#include "netio.hpp"

#include "processlogo.hpp"
// #include "processlogin.hpp"

#include <SDL2/SDL.h>
#include <atomic>
#include "cachequeue.hpp"

class Game
{
    private:
        double m_FPS;
        double m_ServerDelay;
        double m_NetPackTick;

    private:
        std::atomic<bool> m_LoginOK;

    public:
        Game();
       ~Game();

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
        bool FPSDelay();
        void SwitchProcess(int);
        void SwitchProcess(int, int);

    public:
        void InitASIO();
        void PollASIO();
        void StopASIO();

    private:
        void OnServerMessage(uint8_t, const uint8_t *, size_t);

    private:
        void Net_PING     (const uint8_t *, size_t);
        void Net_LOGINOK  (const uint8_t *, size_t);
        void Net_CORECORD (const uint8_t *, size_t);
        void Net_LOGINFAIL(const uint8_t *, size_t);
        void Net_ACTION   (const uint8_t *, size_t);

    public:
        double GetTimeTick()
        {
            return SDL_GetPerformanceCounter() * 1000.0 / SDL_GetPerformanceFrequency();
        }

    private:
        void EventDelay(double);
        void ProcessEvent();
        void Update(double);
        void Draw();

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

    private:
        // std::string     m_ServerIP;
        // std::string     m_ServerPort;
        NetIO           m_NetIO;

    private:
        // ProcessLogin    *m_ProcessLogin;
        // ProcessLogo     *m_ProcessLogo;

    private:
        Process *m_CurrentProcess;

    private:
        // to get an average delay time in MS for most recent 100 loops
        CacheQueue<double, 100> m_DelayTimeCQ;

    private:
        // TODO
        // won't bother myself to make it in singleton mode
        // but make sure there is only one instance
        SDLDevice   *m_SDLDevice;

    private:
        std::string m_ClipboardBuf;
};
