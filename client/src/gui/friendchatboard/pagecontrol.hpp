#pragma once
#include <utility>
#include <initializer_list>
#include "widget.hpp"

struct PageControl: public Widget
{
    PageControl(
            Widget::VarDir,
            Widget::VarOff,
            Widget::VarOff,

            int,

            std::initializer_list<std::pair<Widget *, bool>>,

            Widget * = nullptr,
            bool     = false);
};

