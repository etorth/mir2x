/*
 * =====================================================================================
 *
 *       Filename: controlboard.hpp
 *        Created: 08/21/2016 04:12:57
 *  Last Modified: 06/25/2017 23:22:39
 *
 *    Description: main control pannel for running game
 *                 this is a fixed-size board, if use game screen size other than
 *                 800 * 600, this board will leave blank on the right
 *
 *                 I realized I can resize part of the frame to fix any screen size
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
#include <cstdint>
#include <functional>

#include "log.hpp"
#include "widget.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"
#include "inputline.hpp"

class ProcessRun;
class ControlBoard: public Widget
{
    private:
        ProcessRun *m_ProcessRun;

    private:
        InputLine m_CmdLine;

    public:
        ControlBoard(int, int, Widget *, bool);
       ~ControlBoard() = default;

    public:
        void Update(double);
        void DrawEx(int, int, int, int, int, int);
        bool ProcessEvent(const SDL_Event &, bool *);

    public:
        // this function is wired, better if I put it in constructor
        // but I don't want to change the interface Widget(int, int, Widget *, boo)
        void Bind(ProcessRun *pRun)
        {
            m_ProcessRun = pRun;
        }

    public:
        void InputLineDone();
};
