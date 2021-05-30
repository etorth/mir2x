/*
 * =====================================================================================
 *
 *       Filename: playerstateboard.hpp
 *        Created: 10/08/2017 19:06:52
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
#include "widget.hpp"
#include "sysconst.hpp"
#include "labelboard.hpp"
#include "protocoldef.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class PlayerStateBoard: public Widget
{
    private:
        struct WearGrid
        {
            int x = 0;
            int y = 0;
            int w = SYS_INVGRIDPW;
            int h = SYS_INVGRIDPH;
            const char8_t *type = nullptr;
        };

    private:
        static constexpr int m_equipCharX =  90;
        static constexpr int m_equipCharY = 200;

    private:
        const std::array<WearGrid, WLG_END> m_gridList;

    private:
        TritexButton m_closeButton;

    private:
        std::vector<TritexButton *> m_elemStateList;

    private:
        ProcessRun *m_processRun;

    public:
        PlayerStateBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void update(double) override;

    public:
        void drawEx(int, int, int, int, int, int) const override;
        void drawWear();

    public:
        bool processEvent(const SDL_Event &, bool) override;

    private:
        void drawItemHoverText(int) const;
};
