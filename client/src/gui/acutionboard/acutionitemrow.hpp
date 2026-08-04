#pragma once
#include <cstddef>
#include "widget.hpp"
#include "textboard.hpp"
#include "gfxshapeboard.hpp"

class AcutionItemList;
class AcutionItemRow final: public Widget
{
    private:
        AcutionItemList *m_itemList;
        const size_t m_rowIndex;

    private:
        GfxShapeBoard m_background;

    private:
        TextBoard m_itemName;
        TextBoard m_seller;
        TextBoard m_timeLeft;
        TextBoard m_price;

    public:
        AcutionItemRow(AcutionItemList *, size_t);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;
};
