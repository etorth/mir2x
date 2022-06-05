#pragma once
#include <cstdint>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "threadpool.hpp"

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

    inline Fl_Color color(size_t index)
    {
        const static Fl_Color s_table[]
        {
            FL_RED,
            FL_GREEN,
            FL_BLUE,
            FL_YELLOW,
            FL_MAGENTA,
            FL_CYAN,
            fl_rgb_color(156, 102,  31),
            fl_rgb_color(255, 127,  80),
            fl_rgb_color(176,  23,  31),
            fl_rgb_color(116,   0,   0),
            fl_rgb_color( 64, 224, 205),
            fl_rgb_color(  8,  46,  84),
            fl_rgb_color( 34, 139,  34),
            fl_rgb_color(  3, 168, 158),
            fl_rgb_color( 25,  25, 112),
            fl_rgb_color(255, 153,  18),
            fl_rgb_color(237, 145,  33),
            fl_rgb_color( 85, 102,   0),
            fl_rgb_color(128,  42,  42),
            fl_rgb_color(218, 112, 214),
            fl_rgb_color(153,  51, 250),
            fl_rgb_color(160,  82,  45),
        };

        if(index < std::extent_v<decltype(s_table)>){
            return s_table[index];
        }
        return FL_BLACK;
    }

    template<typename Callable> void mtcall(Callable &&f)
    {
        Fl::lock();
        const threadPool::scopeGuard lockGuard([](){ Fl::unlock(); });
        f();
    }
}
