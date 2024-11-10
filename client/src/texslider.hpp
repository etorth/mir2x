#pragma once
#include <cstdint>
#include <SDL2/SDL.h>
#include "totype.hpp"
#include "widget.hpp"
#include "slider.hpp"

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

    public:
        TexSlider(Widget::VarDir,

                Widget::VarOff,
                Widget::VarOff,
                Widget::VarSize,
                Widget::VarSize,

                bool,
                int,

                std::function<void(float)>,

                Widget * = nullptr,
                bool     = false);

    public:
        void resizeWidth(int width)
        {
            if(width >= 0){
                m_w = width;
            }
        }

        void resizeHeight(int height)
        {
            if(height >= 0){
                m_h = height;
            }
        }

    public:
        void drawEx(int, int, int, int, int, int) const override;
};
