#pragma once
#include "widget.hpp"
#include "itemflex.hpp"
#include "sliderbase.hpp"
#include "imageboard.hpp"
#include "marginwrapper.hpp"
#include "gfxshapeboard.hpp"
#include "gfxresizeboard.hpp"

class GfxDebugBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            bool hflip  = false;
            bool vflip  = false;
            int  rotate = 0;

            Widget::WADPair parent {};
        };

    private:
        GfxShapeBoard m_bg;

    private:
        ImageBoard m_img;

    private:
        Widget          m_srcWidget;
        Widget                  m_imgCanvas;
        MarginWrapper                   m_imgContainer;
        GfxShapeBoard                  m_imgFrame;

        SliderBase              m_imgResizeHSlider;
        SliderBase              m_imgResizeVSlider;

        SliderBase              m_cropHSlider_0;
        SliderBase              m_cropHSlider_1;
        SliderBase              m_cropVSlider_0;
        SliderBase              m_cropVSlider_1;

        TextBoard               m_texSize;
        TextBoard               m_imgSize;
        TextBoard               m_roiInfo;
        TextBoard               m_resizeInfo;
        TextBoard               m_marginInfo;

    private:
        Widget          m_dstWidget;
        Widget                  m_dstCanvas;
        GfxResizeBoard                  m_resizeBoard;

        SliderBase              m_marginHSlider_0;
        SliderBase              m_marginHSlider_1;
        SliderBase              m_marginVSlider_0;
        SliderBase              m_marginVSlider_1;

    public:
        GfxDebugBoard(GfxDebugBoard::InitArgs);

    private:
        bool checkResizeW(float, float) const;
        bool checkResizeH(float, float) const;

    public:
        Widget::ROI getROI() const; // take img's DIR_UPLEFT as (0, 0)
};
