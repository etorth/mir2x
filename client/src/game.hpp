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
        void RunASIO();
        void OperateHC(uint8_t);

    private:
        void OnPing();
        void OnLoginOK();

    private:
        double GetTimeMS();
        void   EventDelay(double);
        void   ProcessEvent();
        void   Update(double);
        void   Draw();

    public:
        template<typename... U> void Send(U&&... u)
        {
            m_NetIO.Send(std::forward<U>(u)...);
        }

    private:
        NetIO   m_NetIO;

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
