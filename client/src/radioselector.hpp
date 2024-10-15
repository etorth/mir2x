#pragma once
#include <cstdint>
#include <initializer_list>
#include "widget.hpp"
#include "tritexbutton.hpp"

    // |<------width------->|
    //        gap
    //     ->|   |<-
    // +-----+---+----------+              -
    // |     |   |          |              ^
    // | ( ) |   |  Widget  | |            |
    // |     |   |          | v            |
    // +-----+---+----------+ -            |
    // |                    | item space   +- height
    // +-----+---+----------+ -            |
    // |     |   |          | ^            |
    // |     |   |          | |            |
    // | (x) |   |  Widget  |              |
    // |     |   |          |              |
    // |     |   |          |              v
    // +-----+---+----------+              -
    //
    // size in auto-scaling mode
    //

class RadioSelector: public Widget
{
    private:
        const int m_gap;
        const int m_itemSpace;

    public:
        RadioSelector(Widget::VarDir,

                Widget::VarOffset,
                Widget::VarOffset,

                std::initializer_list<std::tuple<Widget *, bool>> = {},

                int = 5, // gap
                int = 5, // item space

                Widget * = nullptr,
                bool     = false);

    public:
        void append(Widget *, bool);
};
