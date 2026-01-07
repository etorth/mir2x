#pragma once
#include "widget.hpp"
#include "itemflex.hpp"
#include "inputline.hpp"
#include "gfxdirbutton.hpp"
#include "gfxshapeboard.hpp"

class ValueSelector: public Widget
{
    private:
        struct InputArgs final
        {
            Widget::VarSize w = 0;
            Widget::VarBool enableIME = false;

            Widget::FontConfig font {};
            InputLine::CursorArgs cursor {};

            std::function<void(std::string)> onChange = nullptr;
            std::function<bool(std::string)> validate = nullptr;
        };

        struct ButtonArgs final
        {
            Widget::VarSize w = 0;
        };

        struct InitArgs
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSize h = 0;

            ValueSelector::InputArgs   input {};
            ValueSelector::ButtonArgs button {};

            Widget::VarBool finizeOnChange = true;

            Button::TriggerCBFunc   upTrigger = nullptr;
            Button::TriggerCBFunc downTrigger = nullptr;

            Widget::WADPair parent {};
        };

    private:
        InputLine m_input;

    private:
        GfxDirButton m_up;
        GfxDirButton m_down;

    private:
        ItemFlex m_vflex;
        ItemFlex m_hflex;

    private:
        GfxShapeBoard m_frame;

    public:
        std::string getValue() const
        {
            return m_input.getRawString();
        }

        void setValue(std::string value)
        {
            m_input.setInput(value.c_str());
        }

    public:
        ValueSelector(ValueSelector::InitArgs);
};
