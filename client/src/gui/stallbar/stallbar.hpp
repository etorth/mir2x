#pragma once
#include <functional>
#include "widget.hpp"
#include "textboard.hpp"
#include "imageboard.hpp"
#include "trigfxbutton.hpp"
#include "gfxcenterboard.hpp"
#include "gfxresizeboard.hpp"

class StallBar: public Widget
{
    public:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarStrFunc   nameFunc {};
            std::function<void()> onClick {};

            Widget::WADPair parent {};
        };

    private:
        TextBoard  m_name;
        ImageBoard m_bg;

    private:
        ImageBoard     m_frameImage;
        GfxResizeBoard m_frame;

    private:
        GfxCenterBoard m_gfx;
        TrigfxButton   m_button;

    public:
        StallBar(StallBar::InitArgs);

    private:
        int nameSpaceWidth() const
        {
            return std::max<int>(m_name.w(), 60); // m_name must be initialized
        }
};
