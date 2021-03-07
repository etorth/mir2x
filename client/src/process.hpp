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
    PROCESSID_BEGIN = 1,
    PROCESSID_LOGO  = 1,
    PROCESSID_SYRC,
    PROCESSID_LOGIN,
    PROCESSID_NEW,
    PROCESSID_PWD,
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
        virtual int ID() const = 0;

    public:
        virtual void draw() = 0;
        virtual void update(double) = 0;
        virtual void processEvent(const SDL_Event &) = 0;
};
