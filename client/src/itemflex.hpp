#pragma once
#include <utility>
#include <initializer_list>
#include <concepts>
#include "widget.hpp"
#include "itemalign.hpp"

class ItemFlex: public Widget
{
    #include "itemflex.api.hpp"

    private:
        const bool m_vbox;
        const ItemAlign m_align;

    private:
        Widget *m_canvas;

    private:
        Widget::VarSize m_itemSpace;
        Widget::VarSizeOpt m_fixedEdgeSize;

    public:
        ItemFlex(ItemFlex::InitArgs);

    private:
        int canvasW() const;
        int canvasH() const;

    private:
        const Widget *lastShowChild() const;
};

void ItemFlex::clearItem(std::invocable<const Widget *, bool> auto f)
{
    m_canvas->clearChild(f);
}

void ItemFlex::clearItem()
{
    clearItem([](const Widget *, bool){ return true; });
}

auto ItemFlex::foreachItem(this auto && self, bool forward, auto func)
{
    return self.m_canvas->foreachChild(forward, func);
}

auto ItemFlex::foreachItem(this auto && self, auto func)
{
    return self.foreachItem(true, func);
}
