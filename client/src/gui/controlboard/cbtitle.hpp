#pragma once
#include "widget.hpp"
#include "imageboard.hpp"
#include "texaniboard.hpp"
#include "cblevel.hpp"

//                     |
//                     v
//       +-----+      ---   |
//      /       \      21   v
//  +--/  TITLE  \--+ ---   -
//  | /           \ |  ^    -
//  +---------------+  |    ^
//                          |
//                          +-- UP_HEIGHT_EXTRA: for extra height to show when controlboard is minimized

class ProcessRun;
class CBTitle: public Widget
{
    public:
        constexpr static int UP_HEIGHT = 21;
        constexpr static int UP_HEIGHT_EXTRA = 3;

    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard m_bg;
        TexAniBoard m_arcAni;

    private:
        CBLevel m_level;

    public:
        CBTitle(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                ProcessRun *,

                Widget * = nullptr,
                bool    = true);
};
