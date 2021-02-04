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
#include "labelboard.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class PlayerStatusBoard: public Widget
{
    private:
        static constexpr int m_equipCharX = 100;
        static constexpr int m_equipCharY = 200;

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
