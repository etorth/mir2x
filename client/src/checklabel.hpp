#pragma once
#include <cstdint>
#include "widget.hpp"
#include "sdldevice.hpp"
#include "checkbox.hpp"
#include "labelboard.hpp"

class CheckLabel: public Widget
{
    public:
        using BoolGetter  = CheckBox::BoolGetter;
        using BoolSetter  = CheckBox::BoolSetter;
        using TriggerFunc = CheckBox::TriggerFunc;

    protected:
        struct BoxArgs final
        {
            Widget::VarSizeOpt w = std::nullopt;
            Widget::VarSizeOpt h = std::nullopt;
            Widget::VarU32 color = colorf::RGBA(231, 231, 189, 128);
        };

        struct LabelArgs final
        {
            const char8_t     *text {};
            Widget::FontConfig font {};
        };

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            bool boxFirst = true;
            Widget::VarSizeOpt gap = std::nullopt;

            CheckLabel::  BoxArgs   box {};
            CheckLabel::LabelArgs label {};

            CheckBox::BoolGetter  getter   = nullptr; // self is CheckLabel, not CheckBox
            CheckBox::BoolSetter  setter   = nullptr;
            CheckBox::TriggerFunc onChange = nullptr;

            Widget::WADPair parent {};
        };

    private:
        bool m_enableHoverColor = false;

    private:
        CheckBox m_box;
        LabelBoard m_label;

    public:
        CheckLabel(CheckLabel::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        bool getter() const
        {
            return m_box.getter();
        }

        void setter(bool val)
        {
            m_box.setter(val);
        }

    public:
        void setFocus(bool argFocus) override
        {
            Widget::setFocus(false);
            if(argFocus){
                m_box.setFocus(true);
            }
        }
};
