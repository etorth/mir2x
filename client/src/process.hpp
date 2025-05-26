#pragma once
#include <SDL2/SDL.h>

enum ProcessID: int
{
    PROCESSID_NONE  = 0,
    PROCESSID_BEGIN = 1,
    PROCESSID_LOGO  = 1,
    PROCESSID_SYRC,
    PROCESSID_LOGIN,
    PROCESSID_CREATEACCOUNT,
    PROCESSID_SELECTCHAR,
    PROCESSID_CREATECHAR,
    PROCESSID_CHANGEPASSWORD,
    PROCESSID_RUN,
    PROCESSID_EXIT,
    PROCESSID_END,
};

class Process
{
    public:
        Process() = default;

    public:
        virtual ~Process() = default;

    public:
        virtual int id() const = 0;

    public:
        virtual void draw() const = 0;
        virtual void update(double) = 0;
        virtual void processEvent(const SDL_Event &) = 0;
};
