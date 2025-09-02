#pragma once
#include <vector>
#include "widget.hpp"
#include "shapecropboard.hpp"

class TexAniBoard: public Widget
{
    private:
        uint32_t m_startTexID;
        size_t   m_frameCount;
        size_t   m_fps;

    private:
        bool m_fadeInout;
        bool m_loop;

    private:
        double m_accuTime = 0;

    private:
        ShapeCropBoard m_cropArea;

    public:
        TexAniBoard(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                uint32_t,
                size_t,
                size_t,

                bool,
                bool = true,

                Widget * = nullptr,
                bool     = false);

    public:
        void update(double fUpdateTime) override
        {
            m_accuTime += fUpdateTime;
        }

    private:
        std::tuple<int, uint8_t> getDrawFrame() const;
};
