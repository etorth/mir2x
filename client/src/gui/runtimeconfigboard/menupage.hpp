#pragma once
#include <tuple>
#include <initializer_list>
#include "widget.hpp"
#include "gfxshapeboard.hpp"

class MenuPage: public Widget
{
    private:
        GfxShapeBoard m_tabHeaderBg;

    private:
        Widget *m_selectedHeader = nullptr;

    public:
        MenuPage(dir8_t,
                int,
                int,

                Widget::VarSizeOpt,
                int,

                std::initializer_list<std::tuple<const char8_t *, Widget *, bool>>,

                Widget * = nullptr,
                bool     = false);
};
