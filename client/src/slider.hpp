/*
 * =====================================================================================
 *
 *       Filename: slider.hpp
 *        Created: 08/12/2015 09:59:15
 *    Description:
 *
 *                      +---+
 *                +-----|   |----------------+
 *                |     |   |                |
 *                +-----|   |----------------+
 *                      +---+    ^
 *                        ^      |
 *                        |      +-------------- chute
 *                        +--------------------- slider
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
#include <functional>
#include "widget.hpp"
#include "bevent.hpp"

class Slider: public Widget
{
    private:
        bool m_hslider = true;

    protected:
        int m_sliderW = 0;
        int m_sliderH = 0;
        int m_sliderState = BEVENT_OFF;

    private:
        float m_value = 0.0f;

    private:
        std::function<void(float)> m_onChanged;

    public:
        Slider(dir8_t dir, int x, int y, int w, int h, std::function<void(float)> onChanged, bool hslider, int sliderW, int sliderH, Widget *parent = nullptr, bool autoDelete = false)
            : Widget(dir, x, y, w, h, parent, autoDelete)
            , m_hslider(hslider)
            , m_sliderW(sliderW)
            , m_sliderH(sliderH)
            , m_onChanged(std::move(onChanged))
        {}

    public:
        bool processEvent(const SDL_Event &, bool) override;

    public:
        void setValue(float value)
        {
            m_value = std::min<float>(1.0, std::max<float>(0.0, value));
        }

        float getValue() const
        {
            return m_value;
        }

    public:
        void addValue(float diff)
        {
            setValue(m_value + diff);
        }

    protected:
        float pixel2Value(int pixel) const
        {
            return pixel * 1.0f / (m_hslider ? w() : h());
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
};
