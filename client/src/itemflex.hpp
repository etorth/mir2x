#pragma once
#include <utility>
#include <initializer_list>
#include "widget.hpp"
#include "itemalign.hpp"

class ItemFlex: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSizeOpt fixed = std::nullopt; // the other side to flexible edge

            bool v = true;
            ItemAlign align = ItemAlign::UPLEFT;

            Widget::VarSize itemSpace = 0;

            std::initializer_list<std::pair<Widget *, bool>> childList {};
            Widget::WADPair parent {};
        };

    private:
        const bool m_vbox;
        const ItemAlign m_align;

    private:
        Widget *m_canvas;
        Widget::VarSize m_itemSpace;
        Widget::VarSizeOpt m_fixedEdgeSize;

    public:
        ItemFlex(ItemFlex::InitArgs);

    public:
        void addItem(Widget *, bool);

    private:
        int canvasW() const;
        int canvasH() const;
};
