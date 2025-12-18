#pragma once
#include "widget.hpp"
#include "imageboard.hpp"
#include "tritexbutton.hpp"
#include "gfxresizeboard.hpp"

// ProgUse 0X00000450.PNG
// for corners are identical squares of size 58 x 58
// left side squares can be smaller but use same size for simplicity
//
//                       +------+
//                       |      |
//                       v      |
// +=--+---------------+--=+    |
// |   |               |   |<---+--- 58 x 58
// +---+---------------+---+
// |   |               |   |
// |   |               |   |
// |   |               |   |
// |   |               |   |
// +---+---------------+---+
// |   |               | O |
// +=--+---------------+--=+ (510 x 468)

class BaseFrameBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSize w = 0;
            Widget::VarSize h = 0;

            Widget::WADPair parent {};
        };

    private:
        static constexpr      int m_cornerSize = 58;
        static constexpr uint32_t m_frameTexID = 0X00000450;

    private:
        ImageBoard m_frame;
        GfxResizeBoard m_frameBoard;

    private:
        TritexButton m_close;

    public:
        BaseFrameBoard(BaseFrameBoard::InitArgs);
};
