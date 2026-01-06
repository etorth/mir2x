#pragma once
#include "widget.hpp"
#include "pullmenu.hpp"
#include "integerselector.hpp"

class FontSelector: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::WADPair parent {};
        };

    private:
        PullMenu        m_widget;
        PullMenu        m_font;
        IntegerSelector m_size;

    public:
        FontSelector(FontSelector::InitArgs);
};
