#pragma once 
#include "sdldevice.hpp"
#include "netio.hpp"

#include "processlogo.hpp"
#include "processlogin.hpp"

#include <SDL2/SDL.h>
#include <atomic>
#include "tokenboard.hpp"

class Game
{
    private:
        std::atomic<bool>   m_LoginOK;


    public:
        Game();
        ~Game();

    public:
        void Init();
        void MainLoop();

    public:
        enum{
            PROCESSID_NULL   = 0,
            PROCESSID_LOGO   = 1,
            PROCESSID_LOGIN  = 2,
            PROCESSID_RUN    = 3,
            PROCESSID_EXIT   = 4,
        };

    public:
        // since the FontexDB and EmoticonDB are local inside of stGame
        void LoadTokenBoard(TokenBoard *, const tinyxml2::XMLDocument *);

    public:
        void SwitchProcess(int, int);
        void RunASIO();
        void ReadHC();

    private:
        void OnPing();
        void OnLoginOK();

    private:
        NetIO   m_NetIO;

    private:
        ProcessLogin    *m_ProcessLogin;
        ProcessLogo     *m_ProcessLogo;

    private:
        int     *m_CurrentProcessID;
        Process *m_CurrentProcess;

    private:
        // TODO
        // won't bother myself to make it in singleton mode
        // but make sure there is only one instance
        SDLDevice   *m_SDLDevice;

    private:
};
