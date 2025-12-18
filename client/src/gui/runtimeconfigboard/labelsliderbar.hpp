#pragma once
#include "widget.hpp"
#include "labelboard.hpp"
#include "gfxcropboard.hpp"
#include "texsliderbar.hpp"

class LabelSliderBar: public Widget
{
    private:
        LabelBoard   m_label;
        GfxCropBoard m_labelCrop;

    private:
        TexSliderBar m_slider;

    public:
        LabelSliderBar(dir8_t,
                int,
                int,

                const char8_t *,
                int, // label width

                int, // slider index
                int, // slider width
                std::function<void(float)>,

                Widget * = nullptr,
                bool     = false);

    public:
        TexSliderBar *getSlider()
        {
            return &m_slider;
        }
};
