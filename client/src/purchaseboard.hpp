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
#include "serdesmsg.hpp"
#include "texvslider.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class PurchaseBoard: public Widget
{
    private:
        uint64_t m_npcUID = 0;

    private:
        uint64_t m_extendedItemID = 0;
        SDSellItem m_sellItem;

    private:
        int m_ext1Page = 0;

    private:
        int m_selected = 0;
        std::vector<uint32_t> m_itemList;

    private:
        TritexButton m_closeButton;
        TritexButton m_selectButton;

    private:
        TritexButton m_closeExt1Button;
        TritexButton m_leftExt1Button;
        TritexButton m_selectExt1Button;
        TritexButton m_rightExt1Button;

    private:
        TritexButton m_closeExt2Button;
        TritexButton m_selectExt2Button;

    private:
        TexVSlider m_slider;

    private:
        ProcessRun *m_processRun;

    public:
        PurchaseBoard(ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void update(double) override;

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    public:
        void loadSell(uint64_t, std::vector<uint32_t>);

    private:
        size_t getStartIndex() const;

    public:
        uint32_t selectedItemID() const;

    private:
        void setExtendedItemID(uint32_t);

    public:
        void setSellItem(SDSellItem);

    private:
        int extendedBoardGfxID() const;

    private:
        int extendedPageCount() const;
};
