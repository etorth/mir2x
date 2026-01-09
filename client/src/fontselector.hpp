#pragma once
#include "widget.hpp"
#include "pullmenu.hpp"
#include "layoutboard.hpp"
#include "gfxshapeboard.hpp"
#include "integerselector.hpp"

class FontSelector: public Widget
{
    private:
        constexpr static int GAP = 10;
        constexpr static int LAYOUT_WIDTH = 410;

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

    private:
        LayoutBoard m_english;
        LayoutBoard m_chinese;

    private:
        ItemFlex m_vflex;
        GfxShapeBoard m_vflexFrame;

    public:
        FontSelector(FontSelector::InitArgs);
};
