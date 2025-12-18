#pragma once
#include <cstdint>
#include "sdldevice.hpp"
#include "widget.hpp"
#include "imageboard.hpp"
#include "gfxshapeboard.hpp"

class CheckBox: public Widget
{
    public:
        using BoolGetter = std::variant<std::nullptr_t,
                                        std::function<bool()>,
                                        std::function<bool(const Widget *)>>;

        using BoolSetter = std::variant<std::nullptr_t,
                                        std::function<void(bool)>,
                                        std::function<void(Widget *, bool)>>;

        using TriggerFunc = std::variant<std::nullptr_t,
                                        std::function<void(bool)>,
                                        std::function<void(Widget *, bool)>>;

    public:
        static bool evalBoolGetter (const CheckBox::BoolGetter  &, const Widget *);
        static void evalBoolSetter (      CheckBox::BoolSetter  &,       Widget *, bool);
        static void evalTriggerFunc(      CheckBox::TriggerFunc &,       Widget *, bool);

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSizeOpt w = std::nullopt;
            Widget::VarSizeOpt h = std::nullopt;

            Widget::VarU32 color = colorf::RGBA(231, 231, 189, 128);

            CheckBox::BoolGetter  getter   = nullptr; // use m_innVal if getter is not provided
            CheckBox::BoolSetter  setter   = nullptr;
            CheckBox::TriggerFunc onChange = nullptr;

            Widget::WADPair parent {};
        };

    private:
        bool m_innVal = false;

    private:
        Widget::VarU32 m_color;

    private:
        CheckBox::BoolGetter  m_valGetter;
        CheckBox::BoolSetter  m_valSetter;
        CheckBox::TriggerFunc m_valOnChange;

    private:
        ImageBoard m_img;

    private:
        GfxShapeBoard m_box;

    public:
        CheckBox(CheckBox::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void setColor(Widget::VarU32 color)
        {
            m_color = std::move(color);
        }

    public:
        void toggle();

    public:
        bool getter(    ) const;
        void setter(bool);
};
