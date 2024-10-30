#pragma once
#include <vector>
#include <functional>
#include <initializer_list>
#include "widget.hpp"
#include "itembox.hpp"
#include "marginwrapper.hpp"
#include "shapeclipboard.hpp"

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
        ItemBox m_canvas; // holding all menu items
        MarginWrapper m_wrapper;

    private:
        ShapeClipBoard m_background;
        ShapeClipBoard m_frame;

    public:
        MenuBoard(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                Widget::VarSize,
                std::array<int, 4> = {},

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

    // public:
    //     bool processEvent(const SDL_Event &, bool) override;
};
