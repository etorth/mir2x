#pragma once
#include <vector>
#include <functional>
#include <initializer_list>
#include "widget.hpp"
#include "itemflex.hpp"
#include "marginwrapper.hpp"
#include "shapecropboard.hpp"

class MenuBoard: public Widget
{
    private:
        const int m_itemSpace;
        const int m_separatorSpace;

    private:
        std::vector<std::pair<Widget *, bool>> m_itemList;

    private:
        std::function<void(Widget *)> m_onClickMenu;

    private:
        ItemFlex m_canvas; // holding all menu items
        MarginWrapper m_wrapper;

    private:
        ShapeCropBoard m_background;

    public:
        MenuBoard(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                Widget::VarSize,
                std::array<int, 4> = {},

                int = 0,
                int = 0,
                int = 0,

                std::initializer_list<std::tuple<Widget *, bool, bool>> = {},
                std::function<void(Widget *)> = nullptr,

                Widget * = nullptr,
                bool     = false);

    private:
        int upperItemSpace(const Widget *) const; // separator space not included
        int lowerItemSpace(const Widget *) const; // ...

    public:
        void appendMenu(Widget *, bool, bool);
};
