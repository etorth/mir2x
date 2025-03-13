#pragma once
#include <initializer_list>
#include "widget.hpp"
#include "buttonbase.hpp"

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

class MenuButton: public Widget
{
    private:
        const std::array<int, 4> m_margin;

    private:
        Widget *m_gfxWidget;
        Widget *m_menuBoard;

    private:
        ButtonBase m_button;

    public:
        MenuButton(dir8_t,
                int,
                int,

                std::pair<Widget *, bool>,
                std::pair<Widget *, bool>,

                std::array<int, 4> = {},

                Widget * = nullptr,
                bool     = false);

    private:
        void updateMenuButtonSize();

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool) override;
};
