/*
 * =====================================================================================
 *
 *       Filename: purchasecountboard.hpp
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
#include <vector>
#include <cstdint>
#include "widget.hpp"
#include "inputline.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class PurchaseCountBoard: public Widget
{
    private:
        InputLine m_input;

    private:
        TritexButton m_yesButton;
        TritexButton m_nopButton;

    private:
        ProcessRun *m_processRun;

    public:
        PurchaseCountBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void update(double) override;

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    private:
        void inputLineDone();

    public:
        void clear()
        {
            m_input.clear();
        }
};
