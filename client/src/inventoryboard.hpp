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
#include <string>
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
        const int m_invOpButtonX = 295;
        const int m_invOpButtonY = 475;

    private:
        int m_invOpCost = -1;
        int m_selectedIndex = -1;

    private:
        SDStartInvOp m_sdInvOp;

    private:
        WMDAniBoard m_wmdAniBoard;

    private:
        TexVSlider m_slider;

    private:
        TritexButton m_sortButton;
        TritexButton m_closeButton;

    private:
        TritexButton m_sellButton;
        TritexButton m_lockButton;
        TritexButton m_repairButton;

    private:
        ProcessRun *m_processRun;

    public:
        InventoryBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    private:
        void drawGold() const;
        void drawInvOpCost() const;
        void drawInvOpTitle() const;
        void drawItem(int, int, size_t, const PackBin &, uint32_t) const;

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

    private:
        void packBinConsume(const PackBin &);

    public:
        void clearInvOp();
        void startInvOp(SDStartInvOp);
        void setInvOpCost(int, uint32_t, uint32_t, size_t);

    private:
        void commitInvOp();

    public:
        void removeItem(uint32_t, uint32_t, size_t);

    private:
        static std::u8string typeListString(const std::vector<std::u8string> &typeList);
};
