#pragma once
#include <tuple>
#include <string>
#include "mathf.hpp"
#include "widget.hpp"
#include "menuboard.hpp"
#include "imageboard.hpp"
#include "gfxresizeboard.hpp"
#include "menubutton.hpp"
#include "checklabel.hpp"
#include "labelboard.hpp"
#include "texslider.hpp"
#include "tritexbutton.hpp"
#include "trigfxbutton.hpp"
#include "texsliderbar.hpp"
#include "gfxshapeboard.hpp"
#include "baseframeboard.hpp"
#include "textinput.hpp"
#include "pullmenu.hpp"
#include "labelsliderbar.hpp"
#include "menupage.hpp"

class ProcessRun;
class RuntimeConfigBoard: public Widget
{
    private:
        friend class TextInput;
        friend class PullMenu;
        friend class LabelSliderBar;
        friend class MenuPage;

    private:
        SDRuntimeConfig m_sdRuntimeConfig;

    private:
        BaseFrameBoard m_frameBoard;

    private:
        GfxShapeBoard m_leftMenuBackground;
        LayoutBoard    m_leftMenu;

    private:
        PullMenu       m_pageSystem_resolution;
        LabelSliderBar m_pageSystem_musicSlider;
        LabelSliderBar m_pageSystem_soundEffectSlider;
        MenuPage       m_pageSystem;

    private:
        MenuPage m_pageSocial;

    private:
        MenuPage m_pageGameConfig;

    private:
        ProcessRun *m_processRun;

    public:
        RuntimeConfigBoard(int, int, int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    private:
        void reportRuntimeConfig(int);

    public:
        void setConfig(const SDRuntimeConfig &);

    public:
        const SDRuntimeConfig &getConfig() const
        {
            return m_sdRuntimeConfig;
        }

    public:
        void updateWindowSizeLabel(int, int, bool);
};
