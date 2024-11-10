#pragma once
#include <cstdint>
#include <functional>
#include "buttonbase.hpp"

class TritexButton: public ButtonBase
{
    private:
        uint32_t m_texIDList[3];

    private:
        double m_accuBlinkTime = 0.0;
        std::optional<std::tuple<unsigned, unsigned, unsigned>> m_blinkTime = {}; // {off, on} in ms

    private:
        const bool m_alterColor;

    public:
        TritexButton(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                const uint32_t (&)[3],
                const uint32_t (&)[3],

                std::function<void(Widget *)> = nullptr,
                std::function<void(Widget *)> = nullptr,
                std::function<void(Widget *)> = nullptr,

                int = 0,
                int = 0,
                int = 0,
                int = 0,

                bool = true,
                bool = false,
                bool = true,

                Widget * = nullptr,
                bool     = false);

    public:
        void drawEx(int,                 // dst x on the screen coordinate
                    int,                 // dst y on the screen coordinate
                    int,                 // src x on the widget, take top-left as origin
                    int,                 // src y on the widget, take top-left as origin
                    int,                 // size to draw
                    int) const override; // size to draw

    public:
        void setTexID(const uint32_t (&texIDList)[3])
        {
            for(int i: {0, 1, 2}){
                m_texIDList[i] = texIDList[i];
            }
        }

        void setBlinkTime(unsigned offTime, unsigned onTime, unsigned activeTotalTime = 0)
        {
            m_blinkTime = std::make_tuple(offTime, onTime, activeTotalTime);
            m_accuBlinkTime = 0.0;
        }

        void stopBlink()
        {
            m_blinkTime.reset();
            m_accuBlinkTime = 0.0;
        }

    public:
        void update(double fUpdateTime) override
        {
            if(m_blinkTime.has_value()){
                m_accuBlinkTime += fUpdateTime;
                if(const auto activeTotalTime = std::get<2>(m_blinkTime.value()); activeTotalTime > 0 && m_accuBlinkTime > activeTotalTime){
                    m_blinkTime.reset();
                }
            }
        }
};
