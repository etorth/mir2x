#pragma once
#include <cstdint>
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
#include "layoutboard.hpp"
#include "texslider.hpp"
#include "tritexbutton.hpp"
#include "trigfxbutton.hpp"
#include "texsliderbar.hpp"
#include "gfxshapeboard.hpp"
#include "baseframeboard.hpp"
#include "textinput.hpp"
#include "pullmenu.hpp"
#include "sdruntimeconfig.hpp"
#include "labelsliderbar.hpp"
#include "menupage.hpp"

class ProcessRun;
class RuntimeConfigBoard: public Widget
{
    private:
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
        PullMenu       m_pageSystem_ime;
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
        void reportRuntimeConfigRaw(int type, std::string key);

    public:
        template<int INDEX, typename... Args> void reportRuntimeConfig(Args && ... args)
        {
            reportRuntimeConfigRaw(INDEX, SDRuntimeConfigAccessor<INDEX>::keyString(std::forward<Args>(args)...));
        }

        void applyAudioConfig();

    public:
        uint32_t dropItemRule(uint32_t) const;
        void     setDropItemRule(uint32_t, uint32_t, bool);

    public:
        void setConfig(const SDRuntimeConfig &);

    public:
        const SDRuntimeConfig &getConfig() const
        {
            return m_sdRuntimeConfig;
        }

    public:
        void updateWindowSize(std::pair<int, int>, bool);
        void updateIME(int, bool);
};
