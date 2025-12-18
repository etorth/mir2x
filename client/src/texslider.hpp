#pragma once
#include <cstdint>
#include <SDL2/SDL.h>
#include "totype.hpp"
#include "widget.hpp"
#include "sliderbase.hpp"
#include "imageboard.hpp"

class TexSlider: public SliderBase
{
    protected:
        using SliderBase::BarArgs;
        using SliderBase::BarBgWidget;

    private:
        struct InitArgs final
        {
            BarArgs bar {};

            int index = 0;
            float value = 0.0f;

            BarBgWidget bgWidget {};

            Widget::VarUpdateFunc<float> onChange = nullptr;
            Widget::WADPair parent {};
        };

    private:
        struct SliderTexInfo
        {
            // define the texture center
            // some slider textures are not in good shape then not using the (w / 2, h / 2)

            const int ox;
            const int oy;

            const int cover;
            const uint32_t texID;
        };

        constexpr static SliderTexInfo m_sliderTexInfoList []
        {
            { 7,  7, 6, 0X00000080},
            { 8,  8, 7, 0X00000081},
            { 7,  8, 5, 0X00000088},
            {10, 12, 7, 0X00000089},
            {13, 15, 8, 0X0000008A},
        };

    private:
        static constexpr auto getSliderTexInfo(int index)
        {
            fflassert(index >= 0);
            fflassert(index < static_cast<int>(std::size(m_sliderTexInfoList)));
            return m_sliderTexInfoList + index;
        }

    public:
        TexSlider(TexSlider::InitArgs);
};
