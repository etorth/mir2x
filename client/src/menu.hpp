#pragma once
#include <utility>
#include <functional>
#include "widget.hpp"

namespace Menu
{
    struct ItemSize final // size crop of widget in menu
    {
        Widget::VarSizeOpt w = std::nullopt;
        Widget::VarSizeOpt h = std::nullopt;
    };

    using ClickCBFunc = std::variant<std::nullptr_t,
                                     std::function<void(        )>,
                                     std::function<void(Widget *)>>;

    void evalClickCBFunc(const ClickCBFunc &, Widget *);
}
