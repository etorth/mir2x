#pragma once
#include <tuple>
#include <cstdint>
#include <optional>
#include "widget.hpp"
#include "sysconst.hpp"
#include "gfxshapeboard.hpp"

class TexAniBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            uint32_t texAniID = SYS_U32NIL;

            std::optional<size_t> frameCount {}; // not set: use the animation's frame count
            std::optional<size_t> fps        {}; // not set: use the animation's per-frame delays, 0 shows frame 0 only

            bool crossFade = false; // blend current and next frame by alpha
            bool loop      = true;

            Widget::WADPair parent {};
        };

    private:
        const uint32_t m_texAniID;
        const std::optional<size_t> m_frameCount;
        const std::optional<size_t> m_fps;

    private:
        const bool m_crossFade;
        const bool m_loop;

    private:
        double m_accuTime = 0;

    private:
        GfxShapeBoard m_cropArea;

    public:
        TexAniBoard(TexAniBoard::InitArgs);

    public:
        void updateDefault(double fUpdateTime) override
        {
            m_accuTime += fUpdateTime;
        }

    private:
        int frameCount() const;
        double frameDuration(int) const;

    private:
        std::tuple<int, uint8_t> getDrawFrame() const;
};
