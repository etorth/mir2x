#pragma once
#include <functional>
#include "widget.hpp"
#include "labelboard.hpp"
#include "imageboard.hpp"
#include "gfxresizeboard.hpp"
#include "inputline.hpp"

class TextInput: public Widget
{
    private:
        LabelBoard m_labelFirst;
        LabelBoard m_labelSecond;

    private:
        ImageBoard     m_image;
        GfxResizeBoard m_imageBg;

    private:
        InputLine m_input;

    public:
        TextInput(
            dir8_t,
            int,
            int,

            const char8_t *,
            const char8_t *,

            int, // argGapFirst
            int, // argGapSecond

            bool,

            int, // argInputW
            int, // argInputH

            std::function<void()> = nullptr,
            std::function<void()> = nullptr,

            Widget * = nullptr,
            bool     = false);

    public:
        void updateDefault(double fUpdateTime) override
        {
            m_input.update(fUpdateTime);
        }

        bool processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m) override
        {
            return m_input.processEvent(event, valid, m);
        }
};
