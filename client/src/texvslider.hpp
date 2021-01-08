/*
 * =====================================================================================
 *
 *       Filename: texvslider.hpp
 *        Created: 08/12/2015 09:59:15
 *    Description:
 *                 texture slider for texId 0x00000070 and 0x00000080
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <cstdint>
#include "widget.hpp"
#include "slider.hpp"

class TexVSlider: public Slider
{
    private:
        struct SliderTexParam
        {
            int w;

            // define the texture center
            // some slider textures are not in good shape then not using the (w / 2, h / 2)

            int offX;
            int offY;

            int sliderSize;
            int sliderCover;

            uint32_t texID;
        };

    private:
        constexpr static SliderTexParam m_param[]
        {
            {5,  8,  9, 10, 4, 0X00000070},
            {8, 12, 13, 15, 7, 0X00000080},
            {8,  7,  8, 12, 5, 0X05000040},
        };

    private:
        const int m_sliderParamIndex;

    public:
        TexVSlider(int, int, int, int, const std::function<void(float)> &, Widget * parent = nullptr, bool autoDelete = false);

    private:
        static const auto &getParam(int paramIndex)
        {
            return m_param[paramIndex];
        }

        const auto &getSelfParam() const
        {
            return TexVSlider::getParam(m_sliderParamIndex);
        }

    public:
        void resizeHeight(int height)
        {
            if(height >= 0){
                m_h = height;
            }
        }

    public:
        void drawEx(int, int, int, int, int, int) const override;
};
