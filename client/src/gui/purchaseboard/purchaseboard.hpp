#pragma once
#include <vector>
#include <cstdint>
#include "widget.hpp"
#include "serdesmsg.hpp"
#include "texslider.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class PurchaseBoard: public Widget
{
    // NPC sell items in the small box
    // +----------------------------
    // | (19, 15)                (252, 15)
    // |  *-----+----------------*
    // |  |     |                |
    // |  |     |(57, 53)        |
    // |  +-----*----------------+
    // | (19, 57)
    // |  *-----+-----------
    // |  |     |
    // |  |     |
    // |  +-----+-----------
    // |
    // +--------------------

    private:
        constexpr static int m_startX = 19;
        constexpr static int m_startY = 15;

        constexpr static int m_boxW = 57 - 19;
        constexpr static int m_boxH = 53 - 15;

        constexpr static int m_lineW = 252 - 19;
        constexpr static int m_lineH =  57 - 15;

    private:
        constexpr static Widget::ROI m_ext1GridArea
        {
            .x = 313,
            .y =  41,
            .w = 152,
            .h = 114,
        };

        constexpr static Widget::ROI m_ext2GridArea
        {
            .x = 303,
            .y =  16,

            .w = m_boxW,
            .h = m_boxH,
        };

        constexpr static Widget::ROI m_ext2PriceArea
        {
            .x = 350,
            .y =  16,

            .w = 150,
            .h = m_boxH,
        };

    private:
        ProcessRun *m_processRun;

    private:
        uint64_t m_npcUID = 0;

    private:
        int m_ext1Page = 0;
        int m_ext1PageGridSelected = -1;

    private:
        SDSellItemList m_sdSellItemList;

    private:
        int m_selected = 0;
        std::vector<uint32_t> m_itemList;

    private:
        ImageBoard m_bg;

    private:
        TritexButton m_buttonClose;
        TritexButton m_buttonSelect;

    private:
        TritexButton m_buttonExt1Close;
        TritexButton m_buttonExt1Left;
        TritexButton m_buttonExt1Select;
        TritexButton m_buttonExt1Right;

    private:
        TritexButton m_buttonExt2Close;
        TritexButton m_buttonExt2Select;

    private:
        TexSlider m_slider;

    public:
        PurchaseBoard(ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void loadSell(uint64_t, std::vector<uint32_t>);

    private:
        size_t getStartIndex() const;

    public:
        uint32_t selectedItemID() const;

    private:
        void setExtendedItemID(uint32_t);

    public:
        void onBuySucceed(uint64_t, uint32_t, uint32_t);
        void setSellItemList(SDSellItemList);

    private:
        int extendedBoardGfxID() const; // Ext1: unpackable, Ext2: packable

    private:
        int extendedPageCount() const;

    private:
        int getExt1PageGrid(int, int) const;
        static std::tuple<int, int, int, int> getExt1PageGridLoc(int, int);

    private:
        void drawExt1(Widget::ROIMap) const;
        void drawExt2(Widget::ROIMap) const;
        void drawExt1GridHoverText(int) const;

    private:
        std::tuple<uint32_t, uint32_t> getExtSelectedItemSeqID() const;

    private:
        size_t getItemPrice(int) const;

    private:
        void drawItemInGrid(const char8_t *, uint32_t, int, int, Widget::ROIMap) const;

    private:
        static SDL_Texture *getItemTexture(const char8_t *, uint32_t);
};
