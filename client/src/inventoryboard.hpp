/*
 * =====================================================================================
 *
 *       Filename: inventoryboard.hpp
 *        Created: 10/08/2017 19:06:52
 *    Description: inventory class
 *                 server won't store the item place
 *                 it only stores the item weight in total
 *
 *                 client can use different strategy to store them
 *                 player has max weight to hold only
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
#include "texvslider.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class InventoryBoard: public Widget
{
    private:
        const int m_invGridX0 = 18;
        const int m_invGridY0 = 59;

    private:
        LabelBoard m_opNameBoard;
        WMDAniBoard m_wmdAniBoard;

    private:
        TexVSlider m_slider;

    private:
        TritexButton m_closeButton;

    private:
        int m_selectedPackBinIndex = -1;

    private:
        ProcessRun *m_processRun;

    public:
        InventoryBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    private:
        void drawGold() const;
        void drawItem(int, int, size_t, bool, bool, const PackBin &) const;

    public:
        void update(double) override;

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    private:
        std::string getGoldStr() const;

    private:
        size_t getStartRow() const;
        size_t getRowCount() const;

    private:
        int getPackBinIndex(int, int) const;
        std::tuple<int, int> getInvGrid(int, int) const;

    private:
        void drawItemHoverText(const PackBin &) const;

    public:
        uint32_t getGrabbedItemID() const;
};
