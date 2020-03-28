/*
 * =====================================================================================
 *
 *       Filename: process.hpp
 *        Created: 08/12/2015 09:59:15
 *    Description: public API for class client only
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
    PROCESSID_NEW   = 4,
    PROCESSID_PWD   = 5,
    PROCESSID_RUN   = 6,
    PROCESSID_EXIT  = 7,
    PROCESSID_MAX   = 8,
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
        virtual void processEvent(const SDL_Event &) = 0;
};
