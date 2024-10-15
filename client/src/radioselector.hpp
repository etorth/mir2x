#pragma once
#include <cstdint>
#include <functional>
#include <initializer_list>
#include "widget.hpp"
#include "imageboard.hpp"
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

    private:
        std::function<void(Widget *, bool)> m_onChange;

    private:
        ImageBoard m_imgOff;
        ImageBoard m_imgOn;
        ImageBoard m_imgDown;

    public:
        RadioSelector(Widget::VarDir,

                Widget::VarOffset,
                Widget::VarOffset,

                int = 5, // gap
                int = 5, // item space

                std::initializer_list<std::tuple<Widget *, bool>> = {},

                std::function<void(Widget *, bool)> = nullptr,

                Widget * = nullptr,
                bool     = false);

    public:
        void append(Widget *, bool);
};
