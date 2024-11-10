#pragma once
#include <array>
#include <string>
#include <unordered_map>
#include "widget.hpp"
#include "labelboard.hpp"
#include "layoutboard.hpp"
#include "trigfxbutton.hpp"
#include "shapeclipboard.hpp"

struct ChatItemRef: public Widget
{
    //  ->|                              |<------ WIDTH = MARGIN * 2 + message.w()
    //  ->| |<----------------------------------- MARGIN
    //  ->||<------------------------------------ CORNER
    //    /------------------------------\  -
    //    | +----------------------+     |  |
    //    | |        message       | (x) |  +---- HEIGHT = MARGIN * 2 + message.h()
    //    | +----------------------+     |  |
    //    \------------------------------/  -
    //                           ->| |<---------- BUTTON_MARGIN
    //                              ->||<-------- BUTTON_R
    //                               ->| |<------ BUTTON_MARGIN

    constexpr static int MARGIN = 3;
    constexpr static int CORNER = 3;

    constexpr static int BUTTON_R      = 5;
    constexpr static int BUTTON_MARGIN = 5;

    ShapeClipBoard background; // round corner rectangle

    LabelBoard     cross;
    ShapeClipBoard crossBg; // round cover under x
    Widget         crossButtonGfx;
    TrigfxButton   crossButton;

    LayoutBoard message;

    ChatItemRef(dir8_t,
            int,
            int,
            int, // max width

            bool, // force max width
            bool, // show x button

            std::string,

            Widget * = nullptr,
            bool     = false);
};
