#pragma once
#include <cstddef>
#include <functional>
#include <tuple>
#include <vector>
#include "pack2d.hpp"
#include "texslider.hpp"
#include "widget.hpp"

class DirectTradeItemList final: public Widget
{
    public:
        struct ClickEvent final
        {
            int packBinIndex = -1;
            int gridX = -1;
            int gridY = -1;
        };

    public:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            std::function<void(ClickEvent)> onClick {};
            Widget::WADPair parent {};
        };

    private:
        constexpr static int m_gridX = 0;
        constexpr static int m_gridY = 8;

        constexpr static int m_gridW = 5;
        constexpr static int m_gridH = 5;

    private:
        std::function<void(ClickEvent)> m_onClick;
        std::vector<PackBin> m_packBinList;

        TexSlider m_slider;
        mutable int m_hoveredIndex = -1;

    public:
        explicit DirectTradeItemList(DirectTradeItemList::InitArgs);

    public:
        void drawDefault(Widget::ROIMap) const override;
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        std::vector<SDItem> itemList() const;
        void setItemList(std::vector<SDItem>);
        bool addItem(SDItem, int = -1, int = -1);

        size_t itemCount() const
        {
            return m_packBinList.size();
        }

        PackBin takeItem(size_t);
        std::vector<SDItem> takeItemList();

        void clear();

    public:
        const SDItem *hoveredItem() const;

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
