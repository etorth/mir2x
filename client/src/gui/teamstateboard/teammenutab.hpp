#pragma once
#include <functional>
#include "widget.hpp"
#include "textboard.hpp"
#include "trigfxbutton.hpp"
#include "gfxshapeboard.hpp"

class TeamMenuTab: public TrigfxButton
{
    public:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            int w = 0;
            int h = 0;

            Widget::VarStrFunc label {};
            Widget::VarBool selected = false;

            std::function<void()> onClick = nullptr;

            Widget::InstAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        Widget::VarBool m_selected;

    private:
        Widget        m_gfx;
        GfxShapeBoard m_background;
        TextBoard     m_label;

    public:
        explicit TeamMenuTab(TeamMenuTab::InitArgs);

    public:
        bool selected() const
        {
            return Widget::evalBool(m_selected, this);
        }
};
