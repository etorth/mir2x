/*
 * =====================================================================================
 *
 *       Filename: playerstatusboard.hpp
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
#include "tritexbutton.hpp"

class ProcessRun;
class PlayerStatusBoard: public Widget
{
    private:
        struct WearGrid
        {
            int x = 0;
            int y = 0;
            int w = SYS_INVGRIDPW;
            int h = SYS_INVGRIDPH;
        };

        enum WearGridIndex: int
        {
            GRID_NONE  = 0,
            GRID_BEGIN = 1,
            GRID_DRESS = 1,
            GRID_HELMET,
            GRID_WEAPON,
            GRID_SHOES,
            GRID_NECKLACE,
            GRID_ARMRING_L,
            GRID_ARMRING_R,
            GRID_RING_L,
            GRID_RING_R,
            GRID_TORCH,
            GRID_CHARM,
            GRID_END,
        };

    private:
        static constexpr int m_equipCharX =  90;
        static constexpr int m_equipCharY = 200;

    private:
        const std::array<WearGrid, GRID_END> m_gridList;

    private:
        TritexButton m_closeButton;

    private:
        std::vector<TritexButton *> m_elemStatusList;

    private:
        ProcessRun *m_processRun;

    public:
        PlayerStatusBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void update(double) override;

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;
};
