#pragma once
#include <tuple>
#include <functional>
#include "widget.hpp"
#include "menuboard.hpp"
#include "imageboard.hpp"
#include "gfxresizeboard.hpp"
#include "labelboard.hpp"
#include "trigfxbutton.hpp"
#include "gfxcropboard.hpp"


class PullMenu: public Widget
{
    private:
        LabelBoard   m_label;
        GfxCropBoard m_labelCrop;

    private:
        ImageBoard     m_menuTitleImage;
        GfxResizeBoard m_menuTitleBackground;
        LabelBoard     m_menuTitle;
        GfxCropBoard   m_menuTitleCrop;

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

                std::initializer_list<std::tuple<Widget *, bool, bool>>,
                std::function<void(Widget *)>,

                Widget * = nullptr,
                bool     = false);

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        LabelBoard *getMenuTitle()
        {
            return &m_menuTitle;
        }

    public:
        void setFocus(bool argFocus) override
        {
            if(!argFocus){
                m_menuList.setShow(false);
            }
            return Widget::setFocus(argFocus);
        }
};
