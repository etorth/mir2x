#pragma once
#include <functional>
#include "widget.hpp"
#include "textboard.hpp"
#include "imageboard.hpp"
#include "tritexbutton.hpp"

// stall name shown over a stalling player's head
// layers: background button 0X235, stall name, frame 0X234
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
        TritexButton m_button;
        TextBoard    m_name;
        ImageBoard   m_frame;

    public:
        StallBar(StallBar::InitArgs);
};
