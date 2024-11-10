#pragma once
#include "widget.hpp"
#include "layoutboard.hpp"

struct ChatInputContainer: public Widget
{
    LayoutBoard layout;
    ChatInputContainer(dir8_t,

            int,
            int,

            Widget * = nullptr,
            bool     = false);
};
