#pragma once
#include <vector>
#include "widget.hpp"

class TexAniBoard: public Widget
{
    private:
        size_t m_fps;
        double m_accuTime;

    private:
        bool m_loop;
        bool m_fadeInout;

    private:
        std::vector<uint32_t> m_texSeq;

    public:
        TexAniBoard(dir8_t, int, int, uint32_t, size_t, size_t, bool, bool loop = true, Widget * pwidget = nullptr, bool autoDelete = false);

    public:
        void update(double) override;

    public:
        void drawEx(int, int, int, int, int, int) const override;

    private:
        std::tuple<int, uint8_t> getDrawFrame() const;
};
