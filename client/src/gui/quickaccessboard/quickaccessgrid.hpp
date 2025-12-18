#pragma once
#include "widget.hpp"
#include "textboard.hpp"
#include "imageboard.hpp"
#include "gfxshapeboard.hpp"

class ProcessRun;
struct QuickAccessGrid: public Widget
{
    const int slot;
    ProcessRun *proc;

    GfxShapeBoard bg;
    ImageBoard    item;
    TextBoard     count;

    QuickAccessGrid(
            Widget::VarDir,
            Widget::VarInt,
            Widget::VarInt,

            Widget::VarSizeOpt,
            Widget::VarSizeOpt,

            int,
            ProcessRun *,

            Widget * = nullptr,
            bool     = false);
};
