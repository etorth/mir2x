#pragma once
#include "widget.hpp"
#include "gfxshapeboard.hpp"
#include "gfxcropboard.hpp"
#include "imageboard.hpp"
#include "gfxresizeboard.hpp"
#include "tritexbutton.hpp"
#include "texslider.hpp"
#include "layoutboard.hpp"

class ProcessRun;
class ControlBoard;
class CBMiddleExpand: public Widget
{
    private:
        constexpr static int LOG_WINDOW_WIDTH_ORIG = 432;
        constexpr static int CMD_WINDOW_WIDTH_ORIG = 350;

        constexpr static int LOG_WINDOW_HEIGHT_ORIG = 228;
        constexpr static int CMD_WINDOW_HEIGHT      =  40;

        constexpr static int LOG_WINDOW_X =  7;
        constexpr static int LOG_WINDOW_Y = 15;

        constexpr static int CMD_WINDOW_X =  6; // 1 pixel less, interesting
        constexpr static int CMD_WINDOW_Y = 248;

    private:
        friend class ControlBoard;

    private:
        ProcessRun *m_processRun;

    private:
        LayoutBoard &m_logBoard;
        LayoutBoard &m_cmdBoard;

    private:
        int m_cmdBoardCropY = 0;

    private:
        GfxShapeBoard m_bg;

    private:
        ImageBoard     m_bgImgFull;
        GfxResizeBoard m_bgImg;

    private:
        TritexButton m_switchMode;

    private:
        TritexButton m_buttonEmoji;
        TritexButton m_buttonMute;

    private:
        TexSlider m_slider;

    private:
        GfxCropBoard m_logView;
        GfxCropBoard m_cmdView;

    public:
        CBMiddleExpand(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                Widget::VarSizeOpt,

                ProcessRun *,

                Widget * = nullptr,
                bool     = false);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap);

    private:
        int getLogWindowWidth () const { return w() - m_bgImgFull.w() + LOG_WINDOW_WIDTH_ORIG ; }
        int getLogWindowHeight() const { return h() - m_bgImgFull.h() + LOG_WINDOW_HEIGHT_ORIG; }
        int getCmdWindowWidth () const { return w() - m_bgImgFull.w() + CMD_WINDOW_WIDTH_ORIG ; }

    private:
        void onCmdCR();
        void onCmdCursorMove();
};
