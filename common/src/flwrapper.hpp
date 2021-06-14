/*
 * =====================================================================================
 *
 *       Filename: flwrapper.hpp
 *        Created: 02/11/2019 21:54:22
 *    Description: 
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
#include <FL/fl_draw.H>

namespace fl_wrapper
{
    class enable_color final
    {
        private:
            Fl_Color m_color;

        public:
            enable_color(Fl_Color color)
                : m_color(fl_color())
            {
                fl_color(color);
            }

            enable_color(uint8_t r, uint8_t g, uint8_t b)
                : m_color(fl_color())
            {
                fl_color(fl_rgb_color(r, g, b));
            }

        public:
            ~enable_color()
            {
                fl_color(m_color);
            }
    };
}
