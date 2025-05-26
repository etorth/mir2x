#pragma once
#include <cstdint>
#include "widget.hpp"
#include "texslider.hpp"
#include "imageboard.hpp"
#include "gfxcropboard.hpp"
#include "gfxdupboard.hpp"

class TexSliderBar: public Widget
{
    private:
        ImageBoard m_slotImage;
        ImageBoard m_barImage;

    private:
        GfxCropBoard m_slotCropLeft;
        GfxCropBoard m_slotCropMiddle;
        GfxCropBoard m_slotCropRight;

    private:
        GfxDupBoard m_slotMidCropDup;
        GfxDupBoard m_barCropDup;

    private:
        TexSlider m_slider;

    public:
        TexSliderBar(dir8_t,
                int,
                int,
                int,

                bool,
                int,

                std::function<void(float)>,

                Widget * = nullptr,
                bool     = false);

    public:
        float getValue() const
        {
            return m_slider.getValue();
        }

        void setValue(float val, bool triggerCallback)
        {
            m_slider.setValue(val, triggerCallback);
            if(m_slider.hslider()){
                m_barCropDup.setW(to_dround(val * (w() - 6)));
            }
            else{
                m_barCropDup.setH(to_dround(val * (h() - 6)));
            }
        }
};
