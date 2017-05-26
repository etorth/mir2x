/*
 * =====================================================================================
 *
 *       Filename: controlboard.hpp
 *        Created: 08/21/2016 04:12:57
 *  Last Modified: 05/25/2017 00:06:30
 *
 *    Description: main control pannel for running game
 *                 this is a fixed-size board, if use game screen size other than
 *                 800 * 600, this board will leave blank on the right
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
#include "log.hpp"
#include "widget.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"
#include "inputboard.hpp"

#include <cstdint>
#include <functional>

class ControlBoard: public Widget
{
    private:
        InputBoard m_CmdLine;

    public:
        ControlBoard(int, int, Widget *, bool);
       ~ControlBoard() = default;

    public:
        void Update(double);
        void DrawEx(int, int, int, int, int, int);
        bool ProcessEvent(const SDL_Event &, bool *);
};
