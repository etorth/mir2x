#pragma once
#include "widget.hpp"
#include "trigfxbutton.hpp"

// menu button to expand a menu board
// this gui system does not support menubar and submenu
//
// +--------+
// |  Proj  |             <--- menu button
// +--------+----------+
// |  Open     CTRL+O  |  <--- menu board
// +-------------------+
// |  Save     CTRL+S  |
// +-------------------+

class MenuButton: public TrigfxButton
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::WADPair gfxWidget {};
            Widget::WADPair menuBoard {};
            Widget::WADPair    parent {};
        };

    private:
        Widget *m_gfxWidget;
        Widget *m_menuBoard;

    public:
        MenuButton(MenuButton::InitArgs);
};
