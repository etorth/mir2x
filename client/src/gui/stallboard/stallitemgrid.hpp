#pragma once
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vector>
#include <functional>
#include "pack2d.hpp"
#include "widget.hpp"
#include "texslider.hpp"
#include "serdesmsg.hpp"

// 6x6 stall item grid with vertical slider
// keeps a copy of stall items, selection is tracked by (itemID, seqID)
class StallItemGrid final: public Widget
{
    public:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            std::function<void()> onSelect {};
            Widget::WADPair parent {};
        };

    private:
        constexpr static int m_gridW = 6;
        constexpr static int m_gridH = 6;

    private:
        std::function<void()> m_onSelect;

    private:
        std::vector<PackBin> m_packBinList;
        std::vector<size_t>  m_priceList;

    private:
        uint32_t m_selectedItemID = 0;
        uint32_t m_selectedSeqID  = 0;

    private:
        TexSlider m_slider;
        mutable int m_hoveredIndex = -1;

    public:
        explicit StallItemGrid(StallItemGrid::InitArgs);

    public:
        void drawDefault(Widget::ROIMap) const override;
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void setItemList(const std::vector<SDStallItem> &);

    public:
        const SDItem *selectedItem() const;
        size_t selectedPrice() const;
        void clearSelection();

    public:
        const SDItem *hoveredItem() const;
        size_t hoveredPrice() const;

    private:
        int selectedIndex() const;

    private:
        size_t rowCount() const;
        size_t maxStartRow() const;
        size_t startRow() const;

        void setStartRow(size_t);

    private:
        std::tuple<int, int> getGrid(int, int) const;
        int getPackBinIndex(int, int) const;
        void drawItem(int, int, size_t, const PackBin &, uint32_t) const;
};
