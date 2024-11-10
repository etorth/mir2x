#pragma once
#include "widget.hpp"
#include "inputline.hpp"
#include "labelboard.hpp"
#include "layoutboard.hpp"
#include "imageboard.hpp"
#include "gfxcropdupboard.hpp"
#include "friendchatboardconst.hpp"

struct SearchInputLine: public Widget
{
    // o: (0,0)
    // x: (3,3) : fixed by gfx resource border
    //
    //   o==========================+ -
    //   | x+---+ +---------------+ | ^
    //   | ||O、| |xxxxxxx        | | | HEIGHT
    //   | ++---+ +---------------+ | v
    //   +==========================+ -
    //
    //   |<------- WIDTH --------->|
    //      |<->|
    //   ICON_WIDTH
    //
    //   ->||<- ICON_MARGIN
    //
    //       -->| |<-- GAP

    constexpr static int WIDTH = UIPage_MIN_WIDTH - UIPage_MARGIN * 2 - 60;
    constexpr static int HEIGHT = 30;

    constexpr static int ICON_WIDTH = 20;
    constexpr static int ICON_MARGIN = 5;
    constexpr static int GAP = 5;

    ImageBoard image;
    GfxCropDupBoard inputbg;

    ImageBoard icon;
    InputLine  input;
    LabelBoard hint;

    SearchInputLine(Widget::VarDir,

            Widget::VarOff,
            Widget::VarOff,

            Widget * = nullptr,
            bool     = false);
};

