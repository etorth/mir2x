/*
 * =====================================================================================
 *
 *       Filename: process.hpp
 *        Created: 08/12/2015 09:59:15
 *  Last Modified: 12/08/2017 16:05:08
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
    PROCESSID_NONE  = 0,
    PROCESSID_LOGO  = 1,
    PROCESSID_SYRC  = 2,
    PROCESSID_LOGIN = 3,
    PROCESSID_RUN   = 4,
    PROCESSID_EXIT  = 5,
    PROCESSID_MAX   = 6,
};

class Process
{
    public:
        Process() = default;

    public:
        virtual ~Process() = default;

    public:
        virtual int ID() const = 0;

    public:
        virtual void Draw() = 0;
        virtual void Update(double) = 0;
        virtual void ProcessEvent(const SDL_Event &) = 0;
};
