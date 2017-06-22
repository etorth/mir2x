/*
 * =====================================================================================
 *
 *       Filename: controlboard.hpp
 *        Created: 08/21/2016 04:12:57
 *  Last Modified: 06/22/2017 12:10:23
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

class ControlBoard: public Widget
{
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
        void InputLineDone();
};
