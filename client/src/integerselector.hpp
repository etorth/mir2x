#pragma once
#include <optional>
#include "valueselector.hpp"

class IntegerSelector: public ValueSelector
{
    public:
        struct InputArgs final
        {
            Widget::VarSize w = 0;
            Widget::FontConfig font {};
            InputLine::CursorArgs cursor {};

            std::function<void(std::string)> onChange = nullptr;
            std::function<bool(std::string)> validate = nullptr;
        };

        struct ButtonArgs final
        {
            Widget::VarSize w = 0;
        };

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSize h = 0;

            IntegerSelector:: InputArgs  input {};
            IntegerSelector::ButtonArgs button {};

            Button::TriggerCBFunc   upTrigger = nullptr;
            Button::TriggerCBFunc downTrigger = nullptr;

            Widget::VarGetter<std::pair<int, int>> range {};

            Widget::WADPair parent {};
        };

    public:
        IntegerSelector(IntegerSelector::InitArgs);

    public:
        std::optional<int> getInt() const;
};
