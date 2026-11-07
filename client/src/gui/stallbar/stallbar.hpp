#pragma once
#include <functional>
#include "widget.hpp"
#include "textboard.hpp"
#include "imageboard.hpp"
#include "trigfxbutton.hpp"
#include "gfxcenterboard.hpp"
#include "gfxresizeboard.hpp"

// stall name shown over a stalling player's head, width fits the stall name
// layers: background 0X235, stall name, frame 0X234
class StallBar: public Widget
{
    public:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarStrFunc nameFunc {};
            std::function<void()> onClick {};

            Widget::WADPair parent {};
        };

    private:
        ImageBoard m_bg;
        TextBoard  m_name;

    private:
        ImageBoard     m_frameImage;
        GfxResizeBoard m_frame;

    private:
        GfxCenterBoard m_gfx;
        TrigfxButton   m_button;

    public:
        StallBar(StallBar::InitArgs);

    private:
        int nameSpaceWidth() const;
};
