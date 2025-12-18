// +----------------------------- InitArgs::{dir, x, y}
// |
// v     +---+
// *-----|   |----------------+
// |     |   |                |
// +-----|   |----------------+
//       +---+    ^
//         ^      |
//         |      +-------------- bar
//         +--------------------- slider

#pragma once
#include <SDL2/SDL.h>
#include <functional>
#include "widget.hpp"
#include "bevent.hpp"
#include "gfxshapeboard.hpp"
#include "margincontainer.hpp"

class SliderBase: public Widget
{
    protected:
        struct BarArgs final
        {
            // full widget's location is decided by bar position and size
            // slider position and size are relative to bar

            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSize w = 0;
            Widget::VarSize h = 0;

            bool v = true; // vertical bar
        };

        struct SliderArgs final
        {
            // value center of slider may not be at the geometric center
            // can happen when using tex as slider

            // (cx, cy) overlaps with bar geometric center when slider value is 0.5, in pixel-level

            Widget::VarIntOpt cx = std::nullopt;
            Widget::VarIntOpt cy = std::nullopt;

            Widget::VarSize w = 10;
            Widget::VarSize h = 10;
        };

        struct BarBgWidget final
        {
            Widget::VarInt ox = 0; // offset (ox, oy) that overlaps with bar DIR_UPLEFT corner
            Widget::VarInt oy = 0;

            Widget *widget     = nullptr;
            bool    autoDelete = false;
        };

    private:
        struct InitArgs final
        {
            BarArgs bar {};
            SliderArgs slider {};

            float value = 0.0f;
            Widget::VarCheckFunc<float> checkFunc = nullptr;

            BarBgWidget                          bgWidget {};
            MarginContainer::ContainedWidget    barWidget {};
            MarginContainer::ContainedWidget sliderWidget {};

            Widget::VarUpdateFunc<float> onChange = nullptr;
            Widget::WADPair parent {};
        };

    private:
        float m_value;
        int   m_sliderState = BEVENT_OFF;

    private:
        Widget::VarCheckFunc<float> m_checkFunc;

    private:
        std::optional<std::pair<Widget::VarInt, Widget::VarInt>> m_bgOff;

    private:
        const BarArgs m_barArgs;
        const SliderArgs m_sliderArgs;

    private:
        const Widget::VarUpdateFunc<float> m_onChange;

    private:
        MarginContainer m_bar;
        MarginContainer m_slider;

    private:
        GfxShapeBoard m_debugDraw;

    public:
        SliderBase(SliderBase::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        int sliderState() const
        {
            return m_sliderState;
        }

    public:
        bool vbar() const
        {
            return m_barArgs.v;
        }

        float getValue() const
        {
            return m_value;
        }

    public:
        void setValue(float, bool); // force set
        void addValue(float, bool);

    protected:
        float pixel2Value(int) const;

    public:
        Widget::ROI getBarROI(int, int) const;

    public:
        Widget::ROI getSliderROI(int, int) const;
        std::tuple<int, int> getValueCenter(int, int) const;

    protected:
        bool inSlider(int, int, Widget::ROIMap) const;

    public:
        void setBarBgWidget(Widget::VarInt, Widget::VarInt, Widget *, bool);

    private:
        int sliderXAtValueFromBar(float, int) const; // only depends on barArgs and sliderArgs
        int sliderYAtValueFromBar(float, int) const; // ....

    private:
        std::optional<int> bgXFromBar(int) const;
        std::optional<int> bgYFromBar(int) const;

    private:
        int widgetXFromBar(int barX) const { return std::min<int>({barX, sliderXAtValueFromBar(0.0f, barX), bgXFromBar(barX).value_or(INT_MAX)}); }
        int widgetYFromBar(int barY) const { return std::min<int>({barY, sliderYAtValueFromBar(0.0f, barY), bgYFromBar(barY).value_or(INT_MAX)}); }
};
