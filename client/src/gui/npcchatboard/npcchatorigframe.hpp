#pragma once
#include "widget.hpp"
#include "imageboard.hpp"

class NPCChatOrigFrame: public Widget
{
    private:
        ImageBoard m_up;
        ImageBoard m_down;

    public:
        NPCChatOrigFrame(
                Widget::VarDir = DIR_UPLEFT,
                Widget::VarInt = 0,
                Widget::VarInt = 0,

                Widget * = nullptr,
                bool     = false);
};
