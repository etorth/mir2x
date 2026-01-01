#pragma once
#include <vector>
#include <variant>
#include <functional>
#include "widget.hpp"
#include "itembox.hpp"
#include "menuitem.hpp"

class MenuBoard: public Widget
{
    public:
        struct AddItemArgs final
        {
            Widget::WADPair gfxWidget {};
            Widget::WADPair subWidget {};

            Widget::VarBool showIndicator = false;
            Widget::VarBool showSeparator = false;
        };

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSizeOpt fixed = std::nullopt; // margin not included
            Widget::VarMargin margin = {};

            Widget::VarSize         corner = 0;
            Widget::VarSize      itemSpace = 0;
            Widget::VarSize separatorSpace = 0;

            std::vector<MenuBoard::AddItemArgs> itemList {};
            MenuItem::ClickCBFunc onClick = nullptr;

            Widget::WADPair parent {};
        };

    private:
        const Widget::VarSize m_itemSpace;
        const Widget::VarSize m_separatorSpace;

    private:
        MenuItem::ClickCBFunc m_onClickMenu;

    private:
        ItemBox m_canvas; // holding all menu items

    public:
        MenuBoard(MenuBoard::InitArgs);

    public:
        void addMenu(MenuBoard::AddItemArgs);
};
