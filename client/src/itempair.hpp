#pragma once
#include "widget.hpp"
#include "itemalign.hpp"

class ItemPair: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSizeOpt flex = std::nullopt;

            bool v = true;
            ItemAlign align = ItemAlign::UPLEFT;

            Widget::WADPair  first {};
            Widget::WADPair second {};
            Widget::WADPair parent {};
        };

    public:
        ItemPair(ItemPair::InitArgs);

    private:
        void addChild  (Widget *, bool)                                                                         override { throw fflerror("ItemPair::addChild"  ); }
        void addChildAt(Widget *, WidgetTreeNode::VarDir, WidgetTreeNode::VarInt, WidgetTreeNode::VarInt, bool) override { throw fflerror("ItemPair::addChildAt"); }
};
