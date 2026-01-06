#pragma once
#include "valueselector.hpp"

class IntegerSelector: public ValueSelector
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarGetter<std::pair<int, int>> range = std::pair<int, int>(0, 0);

            Widget::WADPair parent {};
        };

    public:
        IntegerSelector(IntegerSelector::InitArgs);
};
