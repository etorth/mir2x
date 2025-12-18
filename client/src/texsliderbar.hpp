#pragma once
#include <cstdint>
#include "widget.hpp"
#include "texslider.hpp"
#include "imageboard.hpp"
#include "gfxdupboard.hpp"
#include "gfxresizeboard.hpp"

// slider uses slot: 0X00000460
//              bar: 0X00000470
//
// slot:
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
class TexSliderBar: public TexSlider
{
    public:
        constexpr static int SLOT_FIXED_EDGE_SIZE = 9;
        constexpr static int  BAR_FIXED_EDGE_SIZE = 5;

    protected:
        using TexSlider::BarArgs;
        using TexSlider::BarBgWidget;

    private:
        struct InitArgs final
        {
            BarArgs bar {};

            int index = 0;
            float value = 0.0f;

            Widget::VarUpdateFunc<float> onChange = nullptr;
            Widget::WADPair parent {};
        };

    private:
        ImageBoard m_imgSlot;
        ImageBoard m_imgBar; // 5 pixels height, horizontal direction is identical

    public:
        Widget m_bg;

    private:
        GfxResizeBoard m_slot;
        GfxDupBoard m_bar;

    public:
        TexSliderBar(TexSliderBar::InitArgs);
};
