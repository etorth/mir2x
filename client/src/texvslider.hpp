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
        struct SliderTexParm
        {
            int w;

            // define the texture center
            // these two slider textures are not in good shape then not using the (w / 2, h / 2)

            int offX;
            int offY;

            int sliderSize;
            int sliderCover;

            uint32_t texID;
        };

    private:
        constexpr static std::array<SliderTexParm, 2> m_parm
        {{
            {5,  8,  9, 10, 4, 0X00000070},
            {8, 12, 13, 15, 7, 0X00000080},
        }};

    private:
        bool m_small;   // true : 0X00000070
                        // false: 0X00000080

    public:
        TexVSlider(int, int, int, bool, Widget * parent = nullptr, bool autoDelete = false);

    private:
        static const auto &getParm(int small)
        {
            return m_parm[small ? 0 : 1];
        }

        const auto &getSelfParm() const
        {
            return TexVSlider::getParm(m_small);
        }

    public:
        void drawEx(int, int, int, int, int, int) override;
};
