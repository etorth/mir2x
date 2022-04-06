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
#include <tuple>
#include <cstdint>
#include "widget.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class QuickAccessBoard: public Widget
{
    private:
        ProcessRun *m_processRun;

    private:
        TritexButton m_buttonClose;

    private:
        constexpr static uint32_t m_texID = 0X00000060;

    public:
        QuickAccessBoard(int, int, ProcessRun *, Widget *pwidget = nullptr, bool autoDelete = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    public:
        static std::tuple<int, int, int, int> getGridLoc(int i)
        {
            if(i >= 0 && i < 6){
                return {17 + 42 * i, 6, 36, 36};
            }
            throw fflerror("invalid quick access grid index: %d", i);
        }

    public:
        void gridConsume(int);
};
