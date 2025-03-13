#pragma once
#include "serdesmsg.hpp"
#include "widget.hpp"
#include "labelboard.hpp"
#include "imageboard.hpp"
#include "shapeclipboard.hpp"
#include "friendchatboardconst.hpp"

struct SearchAutoCompletionItem: public Widget
{
    // o: (0,0)
    // x: (3,3)
    //
    //   o--------------------------+ -
    //   | x+---+ +---------------+ | ^
    //   | ||O、| |label          | | | HEIGHT
    //   | ++---+ +---------------+ | v
    //   +--------------------------+ -
    //   |<-------- WIDTH --------->|
    //      |<->|
    //   ICON_WIDTH
    //
    //   ->||<- ICON_MARGIN
    //
    //       -->| |<-- GAP

    constexpr static int WIDTH = UIPage_MIN_WIDTH - UIPage_MARGIN * 2;
    constexpr static int HEIGHT = 30;

    constexpr static int ICON_WIDTH = 20;
    constexpr static int ICON_MARGIN = 5;
    constexpr static int GAP = 5;

    const bool byID; // when clicked, fill input automatically by ID if true, or name if false
    const SDChatPeer candidate;

    ShapeClipBoard background;

    ImageBoard icon;
    LabelBoard label;

    SearchAutoCompletionItem(Widget::VarDir,

            Widget::VarOff,
            Widget::VarOff,

            bool,
            SDChatPeer,

            const char * = nullptr,

            Widget * = nullptr,
            bool     = false);

    bool processEventDefault(const SDL_Event &, bool) override;
};

