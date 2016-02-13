#pragma once
#include <SDL.h>

class Game;
class Process
{
    public:
        enum{
            PROCESSID_NULL        = 0,
            PROCESSID_LOGO        = 1,
            PROCESSID_SYRC        = 2,
            PROCESSID_LOGIN       = 3,
            PROCESSID_RUN         = 4,
        };
    public:
        Process(int, Game *);
        virtual ~Process();
    public:
        bool RequestQuit();
        bool RequestNewProcess();
        int  ProcessID();
        int  NextProcessID();
    protected:
        bool   m_Quit;
        int    m_ProcessID;
        int    m_NextProcessID;
        Uint32 m_StartTime;
        Uint32 m_FPS;
        Uint32 m_InvokeCount;
        Game  *m_Game;

    public:
        void EventDelay();
        void ClearEvent();
    public:
        virtual void Enter();
        virtual void Exit();
    public:
        virtual void Update();
        virtual void Draw()                   = 0;
        virtual void HandleEvent(SDL_Event *) = 0;
};
