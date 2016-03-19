#pragma once
#include <SDL2/SDL.h>

enum ProcessID: int{
    PROCESSID_NULL        = 0,
    PROCESSID_LOGO        = 1,
    PROCESSID_SYRC        = 2,
    PROCESSID_LOGIN       = 3,
    PROCESSID_RUN         = 4,
};


class Process
{
    public:
    public:
        Process();
        virtual ~Process();

    public:
        bool RequestQuit();
        bool RequestNewProcess();
        int  NextProcessID();

    public:
        virtual int ID() = 0;

    public:
        int    m_NextProcessID;
        double m_TotalTime;
        double m_FPS;
        double m_InvokeCount;
        bool   m_Quit;

    public:
        virtual void Update(double)                  = 0;
        virtual void Draw()                          = 0;
        virtual void ProcessEvent(const SDL_Event &) = 0;
};
