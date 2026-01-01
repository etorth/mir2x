#pragma once
#include "menuitem.hpp"
#include "trigfxbutton.hpp"

// menu button to expand a menu board
// this gui system does not support menubar
//
// +--------+
// |  Proj  |             <--- menu button
// +--------+----------+
// |  Open     CTRL+O  |  <--- menu item --+
// +-------------------+                   |
// |  Save     CTRL+S  |  <--- menu item --+- menu board
// +-------------------+

class MenuButton: public MenuItem
{
    private:
        using ItemSizeArgs = MenuItem::ItemSizeArgs;

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarMargin margin {};
            MenuButton::ItemSizeArgs itemSize {}; // margin not included

            Widget::WADPair gfxWidget {};
            Widget::WADPair subWidget {};

            Widget::VarBool expandOnHover = false;
            Widget::VarU32  bgColor = 0U;

            Widget::WADPair parent {};
        };

    public:
        MenuButton(MenuButton::InitArgs);
};
