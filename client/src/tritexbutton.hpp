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
                dir8_t argDir,
                int argX,
                int argY,

                const uint32_t (& texIDList)[3],
                const uint32_t (&seffIDList)[3],

                std::function<void()> fnOnOverIn  = nullptr,
                std::function<void()> fnOnOverOut = nullptr,
                std::function<void()> fnOnClick   = nullptr,

                int offXOnOver  = 0,
                int offYOnOver  = 0,
                int offXOnClick = 0,
                int offYOnClick = 0,

                bool onClickDone = true,
                bool alterColor  = true,

                Widget *widgetPtr  = nullptr,
                bool    autoDelete = false)
            : ButtonBase
              {
                  argDir,
                  argX,
                  argY,
                  0,
                  0,

                  std::move(fnOnOverIn),
                  std::move(fnOnOverOut),
                  std::move(fnOnClick),

                  seffIDList[0],
                  seffIDList[1],
                  seffIDList[2],

                  offXOnOver,
                  offYOnOver,
                  offXOnClick,
                  offYOnClick,

                  onClickDone,
                  widgetPtr,
                  autoDelete,
              }
            , m_texIDList
              {
                  texIDList[0],
                  texIDList[1],
                  texIDList[2],
              }
            , m_alterColor(alterColor)
        {
            // hide PNGTexDB and SDLDevice
            // query texture size and setup the button size
            initButtonSize();
        }

    public:
        void drawEx(int,                 // dst x on the screen coordinate
                    int,                 // dst y on the screen coordinate
                    int,                 // src x on the widget, take top-left as origin
                    int,                 // src y on the widget, take top-left as origin
                    int,                 // size to draw
                    int) const override; // size to draw
    private:
        void initButtonSize();

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
