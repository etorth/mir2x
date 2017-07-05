/*
 * =====================================================================================
 *
 *       Filename: game.hpp
 *        Created: 08/12/2015 09:59:15
 *  Last Modified: 07/04/2017 20:20:57
 *
 *    Description: public API for class game only
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
#include "cachequeue.hpp"

#include "netio.hpp"
#include "process.hpp"
#include "sdldevice.hpp"

class Game final
{
    private:
        double m_FPS;
        double m_ServerDelay;
        double m_NetPackTick;

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
        NetIO m_NetIO;

    private:
        Process *m_CurrentProcess;

    private:
        std::string m_ClipboardBuf;
};
