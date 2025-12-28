#pragma once
#include "widget.hpp"
#include "imageboard.hpp"
#include "gfxresizeboard.hpp"

//   texID: 0X00000460
//
//   up and down side borders are of 2 pixels:
//
//      1. use 3 pixels in GfxResizeBoard resize ROI then dark/light pixel doesn't repeat
//      2. getInputROI() still uses 2 pixel border for input area inside
//
//   |<-3->|             v
//   +-----------------  -
//   |        border     2
//   |     +-----------  -  -
//   |     |             ^  ^          ---- 1 pixel dark
//   |     |                |          --+
//   |     |                5          --+- 3 pixel gray and can repeat
//   |     |                |          --+
//   |     |                v  v       ---- 1 pixel light
//   |     +-----------     -  -
//   |        border           2
//   +-----------------        -
//                             ^
//

class TexInputBackground: public Widget
{
    public:
        constexpr static int SLOT_FIXED_EDGE_SIZE = 9;

    public:
        constexpr static int MIN_WIDTH  = 6; // because tex has border pixels
        constexpr static int MIN_HEIGHT = 6;

    public:
        static Widget::ROI fromInputROI(bool, Widget::ROI);

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSize w = 0;
            Widget::VarSize h = 0;

            bool v = true;
            Widget::WADPair parent {};
        };

    private:
        ImageBoard m_img;
        GfxResizeBoard m_resize;

    public:
        TexInputBackground(TexInputBackground::InitArgs);

    public:
        bool v() const noexcept
        {
            return m_img.transposed();
        }

    public:
        Widget::ROI getInputROI() const;

    public:
        void setInputSize(Widget::VarSize2D);
        void setInputSize(Widget::VarSize, Widget::VarSize);
};
