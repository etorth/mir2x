#pragma once
#include <utility>
#include <initializer_list>
#include <concepts>
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

    private:
        Widget::VarSize m_itemSpace;
        Widget::VarSizeOpt m_fixedEdgeSize;

    public:
        ItemFlex(ItemFlex::InitArgs);

    public:
        void    addItem(Widget *, bool);
        void removeItem(uint64_t, bool);

    public:
        bool hasShowItem() const;
        void flipItemShow(uint64_t);

    public:
        void buildLayout(); // empty function

    public:
        void clearItem(std::invocable<const Widget *, bool> auto f)
        {
            m_canvas->clearChild(f);
        }

        void clearItem()
        {
            clearItem([](const Widget *, bool){ return true; });
        }

    public:
        auto foreachItem(this auto && self, bool forward, auto func)
        {
            return self.m_canvas->foreachChild(forward, func);
        }

        auto foreachItem(this auto && self, auto func)
        {
            return self.foreachItem(true, func);
        }

    private:
        int canvasW() const;
        int canvasH() const;

    private:
        const Widget *lastShowChild() const;
};
