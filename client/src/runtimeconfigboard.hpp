#pragma once
#include <tuple>
#include <string>
#include "mathf.hpp"
#include "widget.hpp"
#include "menuboard.hpp"
#include "imageboard.hpp"
#include "gfxcutoutboard.hpp"
#include "gfxcropdupboard.hpp"
#include "menubutton.hpp"
#include "checklabel.hpp"
#include "labelboard.hpp"
#include "texslider.hpp"
#include "tritexbutton.hpp"
#include "trigfxbutton.hpp"
#include "texsliderbar.hpp"
#include "shapeclipboard.hpp"
#include "resizableframeboard.hpp"

class ProcessRun;
class RuntimeConfigBoard: public Widget
{
    private:
        class TextInput: public Widget
        {
            private:
                LabelBoard m_labelFirst;
                LabelBoard m_labelSecond;

            private:
                ImageBoard      m_image;
                GfxCropDupBoard m_imageBg;

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
                void update(double fUpdateTime) override
                {
                    m_input.update(fUpdateTime);
                }

                bool processEvent(const SDL_Event &event, bool valid) override
                {
                    return m_input.processEvent(event, valid);
                }
        };

    private:
        class PullMenu: public Widget
        {
            private:
                LabelBoard   m_label;
                GfxCropBoard m_labelCrop;

            private:
                ImageBoard      m_menuTitleImage;
                GfxCropDupBoard m_menuTitleBackground;
                LabelBoard      m_menuTitle;
                GfxCropBoard    m_menuTitleCrop;

            private:
                ImageBoard m_imgOff;
                ImageBoard m_imgOn;
                ImageBoard m_imgDown;

            private:
                TrigfxButton m_button;

            private:
                MenuBoard m_menuList;

            public:
                PullMenu(dir8_t,
                        int,
                        int,

                        const char8_t *, // label text
                        int,             // label width

                        int, // title background width
                        int, // title background height

                        std::initializer_list<std::pair<Widget *, bool>>,
                        std::function<void(Widget *)>,

                        Widget * = nullptr,
                        bool     = false);

            public:
                void drawEx(int, int, int, int, int, int) const override;

            public:
                bool processEvent(const SDL_Event &, bool) override;

            public:
                LabelBoard *getMenuTitle()
                {
                    return &m_menuTitle;
                }
        };

    private:
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

    private:
        class MenuPage: public Widget
        {
            private:
                class TabHeader: public Widget
                {
                    private:
                        LabelBoard m_label;
                        TrigfxButton m_button;

                    public:
                        TabHeader(dir8_t,
                                int,
                                int,

                                const char8_t *,
                                std::function<void(ButtonBase *)>,

                                Widget * = nullptr,
                                bool     = false);
                };

            private:
                ShapeClipBoard m_buttonMask;

            private:
                Widget *m_selectedHeader = nullptr;

            public:
                MenuPage(dir8_t,
                        int,
                        int,

                        Widget::VarSize,
                        int,

                        std::initializer_list<std::tuple<const char8_t *, Widget *, bool>>,

                        Widget * = nullptr,
                        bool     = false);
        };

    private:
        SDRuntimeConfig m_sdRuntimeConfig;

    private:
        ResizableFrameBoard m_frameBoard;

    private:
        ShapeClipBoard m_leftMenuBackground;
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
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

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
