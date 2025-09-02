#pragma once
#include <cstdint>
#include <SDL2/SDL.h>
#include "totype.hpp"
#include "widget.hpp"
#include "slider.hpp"
#include "imageboard.hpp"
#include "shapecropboard.hpp"

class TexSlider: public Slider
{
    private:
        struct SliderTexInfo
        {
            const int w;
            const int h;

            // define the texture center
            // some slider textures are not in good shape then not using the (w / 2, h / 2)

            const int offX;
            const int offY;

            const int cover;
            const uint32_t texID;
        };

    private:
        static const SliderTexInfo &getSliderTexInfo(int index)
        {
            constexpr static SliderTexInfo s_sliderTexInfo []
            {
                { 8,  8,  7,  8, 5, 0X00000080},
                {18, 18,  9,  9, 5, 0X00000081},
                { 5,  5,  8,  9, 4, 0X00000088},
                { 8,  8, 12, 13, 7, 0X00000089},
                { 8,  8, 12, 13, 7, 0X0000008A},
            };

            if(index >= 0 && index < to_d(std::extent_v<decltype(s_sliderTexInfo)>)){
                return s_sliderTexInfo[index];
            }
            throw fflvalue(index);
        }

    private:
        const SliderTexInfo &m_sliderTexInfo;

    private:
        ImageBoard m_image;
        ImageBoard m_cover;

    private:
        ShapeCropBoard m_debugDraw;

    public:
        TexSlider(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                Widget::VarSize,
                Widget::VarSize,

                bool,
                int,

                std::function<void(float)>,

                Widget * = nullptr,
                bool     = false);
};
