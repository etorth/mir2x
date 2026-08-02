#pragma once
#include <cstddef>
#include "textboard.hpp"
#include "trigfxbutton.hpp"

class TextButton: public TrigfxButton
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarStrFunc textFunc {};

            Widget::FontConfig font {};
            Button::SeffIDList seff {};

            Button::OverCBFunc onOverIn  = nullptr;
            Button::OverCBFunc onOverOut = nullptr;

            Button::ClickCBFunc onClick = nullptr;
            Button::TriggerCBFunc onTrigger = nullptr;

            int offXOnOver = 0;
            int offYOnOver = 0;

            int offXOnClick = 0;
            int offYOnClick = 0;

            bool onClickDone = true;
            bool radioMode   = false;

            Widget::InstAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        TextBoard m_text;

    public:
        explicit TextButton(TextButton::InitArgs);
};
