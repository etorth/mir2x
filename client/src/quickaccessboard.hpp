/*
 * =====================================================================================
 *
 *       Filename: quickaccessboard.hpp
 *        Created: 03/28/2020 05:43:45
 *    Description: 
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
#include "widget.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class QuickAccessBoard: public Widget
{
    private:
        ProcessRun *m_proc;

    private:
        TritexButton m_buttonClose;

    private:
        constexpr static uint32_t m_texID = 0X00000060;

    public:
        QuickAccessBoard(int, int, ProcessRun *);

    public:
        void drawEx(int, int, int, int, int, int) override;
        bool processEvent(const SDL_Event &, bool) override;
};
