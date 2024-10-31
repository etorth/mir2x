//       +---+
// +-----|   |----------------+
// |     |   |                |
// +-----|   |----------------+
//       +---+    ^
//         ^      |
//         |      +-------------- chute
//         +--------------------- slider

#pragma once
#include <SDL2/SDL.h>
#include <functional>
#include "mathf.hpp"
#include "widget.hpp"
#include "bevent.hpp"

class Slider: public Widget
{
    private:
        const bool m_hslider = true;

    protected:
        const int m_sliderW = 0;
        const int m_sliderH = 0;

    protected:
        int m_sliderState = BEVENT_OFF;

    private:
        float m_value = 0.0f;

    private:
        const std::function<void(float)> m_onChanged;

    public:
        Slider(dir8_t argDir, int argX, int argY, int argW, int argH, bool hslider, int sliderW, int sliderH, std::function<void(float)> onChanged, Widget *parent = nullptr, bool autoDelete = false)
            : Widget(argDir, argX, argY, argW, argH, {}, parent, autoDelete)
            , m_hslider(hslider)
            , m_sliderW(sliderW)
            , m_sliderH(sliderH)
            , m_onChanged(std::move(onChanged))
        {}

    public:
        bool processEventDefault(const SDL_Event &, bool) override;

    public:
        virtual void setValue(float value, bool triggerCallback)
        {
            if(const auto newValue = mathf::bound<float>(value, 0.0f, 1.0f); newValue != getValue()){
                m_value = newValue;
                if(triggerCallback && m_onChanged){
                    m_onChanged(getValue());
                }
            }
        }

        float getValue() const
        {
            return m_value;
        }

    public:
        void addValue(float diff, bool triggerCallback)
        {
            setValue(m_value + diff, triggerCallback);
        }

    protected:
        float pixel2Value(int pixel) const
        {
            return pixel * 1.0f / std::max<int>(m_hslider ? w() : h(), 1);
        }

    public:
        std::tuple<int, int> getValueCenter() const
        {
            return
            {
                x() + std::lround(( m_hslider ? getValue() : 0.5f) * w()),
                y() + std::lround((!m_hslider ? getValue() : 0.5f) * h()),
            };
        }

        std::tuple<int, int, int, int> getSliderRectangle() const
        {
            const auto [centerX, centerY] = getValueCenter();
            return
            {
                centerX - m_sliderW / 2,
                centerY - m_sliderH / 2,
                m_sliderW,
                m_sliderH,
            };
        }

        bool hslider() const
        {
            return m_hslider;
        }
};
