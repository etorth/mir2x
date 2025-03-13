#pragma once
#include <functional>
#include <SDL2/SDL.h>
#include "widget.hpp"
#include "colorf.hpp"
#include "imageboard.hpp"
#include "tritexbutton.hpp"
#include "gfxcropdupboard.hpp"

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

class ResizableFrameBoard: public Widget
{
    private:
        static constexpr      int m_cornerSize = 58;
        static constexpr uint32_t m_frameTexID = 0X00000450;

    private:
        ImageBoard m_frame;
        GfxCropDupBoard m_frameCropDup;

    private:
        TritexButton m_close;

    public:
        ResizableFrameBoard(dir8_t,
                int,
                int,
                int,
                int,

                Widget * = nullptr,
                bool     = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;
};
