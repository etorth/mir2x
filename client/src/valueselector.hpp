#pragma once
#include "widget.hpp"
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

            ValueSelector::InputArgs   input {};
            ValueSelector::ButtonArgs button {};

            Widget::VarBool finizeOnChange;
        };

    private:
        InputLine m_input;

    private:
        GfxDirButton m_up;
        GfxDirButton m_down;

    public:
        ValueSelector();
};
