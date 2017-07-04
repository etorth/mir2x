/*
 * =====================================================================================
 *
 *       Filename: process.hpp
 *        Created: 08/12/2015 09:59:15
 *  Last Modified: 07/04/2017 14:21:25
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
#include <SDL2/SDL.h>

enum ProcessID: int
{
    PROCESSID_NULL        = 0,
    PROCESSID_LOGO        = 1,
    PROCESSID_SYRC        = 2,
    PROCESSID_LOGIN       = 3,
    PROCESSID_RUN         = 4,
    PROCESSID_EXIT        = 5,
};

class Process
{
    public:
        Process() = default;

    public:
        virtual ~Process() = default;

    public:
        virtual int ID() = 0;

    public:
        virtual void Update(double)                  = 0;
        virtual void Draw()                          = 0;
        virtual void ProcessEvent(const SDL_Event &) = 0;
};
