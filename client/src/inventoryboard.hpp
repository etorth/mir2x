// inventory class
// server won't store the item place
// it only stores the item weight in total
//
// client can use different strategy to store them
// player has max weight to hold only

#pragma once
#include <string>
#include "widget.hpp"
#include "texslider.hpp"
#include "labelboard.hpp"
#include "textboard.hpp"
#include "wmdaniboard.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class InventoryBoard: public Widget
{
    private:
        struct InitArgs final
        {
            int x = 0;
            int y = 0;

            ProcessRun *runProc {};
            Widget::WADPair parent{};
        };

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
        ProcessRun *m_processRun;

    private:
        SDStartInvOp m_sdInvOp;

    private:
        WMDAniBoard m_wmdAniBoard;

    private:
        TexSlider m_slider;

    private:
        TritexButton m_sortButton;
        TritexButton m_closeButton;

    private:
        TritexButton m_invOpButton;
        TextBoard    m_invOpTitle;

    private:
        TextBoard m_goldStr;

    public:
        InventoryBoard(InventoryBoard::InitArgs);

    private:
        void drawGold() const;
        void drawInvOpCost() const;
        void drawInvOpTitle() const;
        void drawItem(int, int, size_t, const PackBin &, uint32_t) const;

    public:
        void updateDefault(double) override;

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

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
