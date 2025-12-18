#pragma once
#include <cstdint>
#include <functional>

#include "widget.hpp"
#include "acbutton.hpp"
#include "texslider.hpp"
#include "layoutboard.hpp"
#include "tritexbutton.hpp"
#include "gfxcropboard.hpp"
#include "alphaonbutton.hpp"
#include "gfxshapeboard.hpp"
#include "gfxresizeboard.hpp"
#include "cbface.hpp"

class ProcessRun;
class ControlBoard;
class CBMiddle: public Widget
{
    public:
        constexpr static int CB_MIDDLE_TEX_HEIGHT = 131;

    private:
        constexpr static int LOG_WINDOW_WIDTH_ORIG = 344;
        constexpr static int CMD_WINDOW_WIDTH_ORIG = 344;

        constexpr static int LOG_WINDOW_HEIGHT = 84;
        constexpr static int CMD_WINDOW_HEIGHT = 15;

        constexpr static int LOG_WINDOW_X =  7;
        constexpr static int LOG_WINDOW_Y = 15;

        constexpr static int CMD_WINDOW_X =   7;
        constexpr static int CMD_WINDOW_Y = 106;

    private:
        friend class ControlBoard;

    private:
        ProcessRun *m_processRun;

    private:
        LayoutBoard &m_logBoard;
        LayoutBoard &m_cmdBoard;

    private:
        int m_cmdBoardCropX = 0;

    private:
        GfxShapeBoard m_bg;

    private:
        CBFace m_face;

    private:
        ImageBoard     m_bgImgFull;
        GfxResizeBoard m_bgImg;

    private:
        TritexButton m_switchMode;

    private:
        TexSlider m_slider;

    private:
        GfxCropBoard m_logView;
        GfxCropBoard m_cmdView;

    public:
        CBMiddle(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,
                Widget::VarSizeOpt,

                ProcessRun *,

                Widget * = nullptr,
                bool     = false);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    private:
        void onCmdCR();
        void onCmdCursorMove();

    private:
        int getLogWindowWidth() const { return w() - m_bgImgFull.w() + LOG_WINDOW_WIDTH_ORIG; }
        int getCmdWindowWidth() const { return w() - m_bgImgFull.w() + CMD_WINDOW_WIDTH_ORIG; }
};
