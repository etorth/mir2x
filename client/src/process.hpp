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
        Process() = default;
        virtual ~Process() = default;

    public:
        virtual int ID() = 0;

    public:
        virtual void Update(double)                  = 0;
        virtual void Draw()                          = 0;
        virtual void ProcessEvent(const SDL_Event &) = 0;
};
