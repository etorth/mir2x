#pragma once
#include <string>
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

    constexpr static int BUTTON_MARGIN = 5;

    constexpr static int BUTTON_R = 6;
    constexpr static int BUTTON_D = ChatItemRef::BUTTON_R * 2 - 1;

    constexpr static uint8_t CROSS_FONT_SIZES[3] {14, 14, 12};

    ShapeClipBoard background; // round corner rectangle

    LabelBoard     cross;
    ShapeClipBoard crossBg;        // round cover under x
    Widget         crossButtonGfx; // merge cross and crossBg to be a single gfx-widget, then use it in TrigfxButton
    TrigfxButton   crossButton;    //

    LayoutBoard message;

    ChatItemRef(
            Widget::VarDir,
            Widget::VarOff,
            Widget::VarOff,
            int, // max width

            bool, // force max width
            bool, // show x button

            std::string,

            Widget * = nullptr,
            bool     = false);
};
