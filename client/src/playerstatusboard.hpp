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
            WEAR_NONE  = 0,
            WEAR_BEGIN = 1,
            WEAR_DRESS = 1,
            WEAR_HELMET,
            WEAR_WEAPON,

            WEAR_GRID_BEGIN,
            WEAR_SHOES = WEAR_GRID_BEGIN,
            WEAR_NECKLACE,
            WEAR_ARMRING_L,
            WEAR_ARMRING_R,
            WEAR_RING_L,
            WEAR_RING_R,
            WEAR_TORCH,
            WEAR_CHARM,
            WEAR_GRID_END,

            WEAR_END = WEAR_GRID_END,
        };

    private:
        static constexpr int m_equipCharX =  90;
        static constexpr int m_equipCharY = 200;

    private:
        const std::array<WearGrid, WEAR_END> m_gridList;

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
        void drawWear();

    public:
        bool processEvent(const SDL_Event &, bool) override;

    private:
        uint32_t getGridItemID(int) const;
};
