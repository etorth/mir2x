#pragma once
#include <cstddef>
#include <cstdint>
#include "widget.hpp"
#include "textboard.hpp"
#include "gfxshapeboard.hpp"

class AcutionItemList;
class AcutionItemRow final: public Widget
{
    private:
        friend class AcutionItemList;

    private:
        constexpr static int m_columnWidth_item   = 108;
        constexpr static int m_columnWidth_seller = 140;
        constexpr static int m_columnWidth_time   =  70;
        constexpr static int m_columnWidth_price  = 142;

    private:
        constexpr static int m_rowW = m_columnWidth_item + m_columnWidth_seller + m_columnWidth_time + m_columnWidth_price;
        constexpr static int m_rowH = 19;

    private:
        constexpr static int m_columnCenter_item   = m_columnWidth_item / 2;
        constexpr static int m_columnCenter_seller = m_columnWidth_item + m_columnWidth_seller / 2;
        constexpr static int m_columnCenter_time   = m_columnWidth_item + m_columnWidth_seller + m_columnWidth_time / 2;
        constexpr static int m_columnCenter_price  = m_columnWidth_item + m_columnWidth_seller + m_columnWidth_time + m_columnWidth_price / 2;

    private:
        AcutionItemList *m_itemList;

    private:
        const size_t m_rowIndex;

    private:
        GfxShapeBoard m_background;

    private:
        TextBoard m_item;
        TextBoard m_seller;
        TextBoard m_time;
        TextBoard m_price;

    public:
        AcutionItemRow(AcutionItemList *, size_t);

    private:
        static uint32_t priceColor(size_t);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;
};
