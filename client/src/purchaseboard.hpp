/*
 * =====================================================================================
 *
 *       Filename: purchaseboard.hpp
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
#include "texvslider.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class PurchaseBoard: public Widget
{
    private:
        uint64_t m_npcUID = 0;
        std::vector<uint32_t> m_itemList;

    private:
        TritexButton m_closeButton;
        TritexButton m_selectButton;

    private:
        TexVSlider m_slider;

    private:
        ProcessRun *m_processRun;

    public:
        PurchaseBoard(ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void update(double) override;
        void drawEx(int, int, int, int, int, int) override;
        bool processEvent(const SDL_Event &, bool) override;

    public:
        void loadSell(uint64_t, std::vector<uint32_t>);
};
