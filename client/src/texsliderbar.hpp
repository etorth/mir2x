#pragma once
#include <cstdint>
#include "widget.hpp"
#include "texslider.hpp"
#include "imageboard.hpp"
#include "gfxdupboard.hpp"
#include "texinputbackground.hpp"

class TexSliderBar: public TexSlider
{
    public:
        constexpr static int BAR_FIXED_EDGE_SIZE = 5;

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
        TexInputBackground m_bg;

    private:
        ImageBoard m_imgBar; // 5 pixels height, horizontal direction is identical
        GfxDupBoard   m_bar;

    public:
        TexSliderBar(TexSliderBar::InitArgs);
};
