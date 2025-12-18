#pragma once
#include "widget.hpp"
#include "gfxresizeboard.hpp"
#include "npcchatorigframe.hpp"

class NPCChatFrame: public Widget
{
    private:
        NPCChatOrigFrame m_frame;

    private:
        GfxResizeBoard m_board;

    public:
        NPCChatFrame(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                Widget::VarSizeOpt,
                Widget::VarSizeOpt,

                Widget * = nullptr,
                bool     = false);
};
