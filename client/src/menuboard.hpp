#pragma once
#include <vector>
#include <variant>
#include <functional>
#include <initializer_list>
#include "widget.hpp"
#include "itembox.hpp"
#include "marginwrapper.hpp"

class MenuBoard: public Widget
{
    public:
        using ClickCBFunc = std::variant<std::nullptr_t, std::function<void()>, std::function<void(Widget *)>>;

    public:
        static void evalClickCBFunc(const ClickCBFunc &cbFunc, Widget *widget)
        {
            std::visit(VarDispatcher
            {
                [      ](const std::function<void(        )> &f){ if(f){ f(      ); }},
                [widget](const std::function<void(Widget *)> &f){ if(f){ f(widget); }},

                [](const auto &){},
            },
            cbFunc);
        }

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSizeOpt fixed = std::nullopt;
            Widget::VarMargin margin = {};

            Widget::VarSize         corner = 0;
            Widget::VarSize      itemSpace = 0;
            Widget::VarSize separatorSpace = 0;

            std::initializer_list<std::tuple<Widget *, bool, bool>> itemList {};
            MenuBoard::ClickCBFunc onClick = nullptr;

            Widget::WADPair parent {};
        };

    private:
        const Widget::VarSize m_itemSpace;
        const Widget::VarSize m_separatorSpace;

    private:
        std::vector<std::pair<Widget *, bool>> m_itemList;

    private:
        MenuBoard::ClickCBFunc m_onClickMenu;

    private:
        ItemBox m_canvas; // holding all menu items
        MarginWrapper m_wrapper;

    public:
        MenuBoard(MenuBoard::InitArgs);

    private:
        int upperItemSpace(const Widget *) const; // separator space not included
        int lowerItemSpace(const Widget *) const; // ...

    public:
        void appendMenu(Widget *, bool, bool);
};
