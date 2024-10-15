#pragma once
#include <cstdint>
#include <functional>
#include <initializer_list>
#include "widget.hpp"
#include "imageboard.hpp"
#include "trigfxbutton.hpp"

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
        Widget *m_selected = nullptr;

    private:
        std::function<const Widget *(const Widget * /* self */                                           )> m_valGetter;
        std::function<void          (      Widget * /* self */, Widget * /* child */                     )> m_valSetter;
        std::function<void          (      Widget * /* self */, Widget * /* child */, bool /* selected */)> m_valOnChange;

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

                std::function<const Widget *(const Widget *                )> = nullptr,
                std::function<void          (      Widget *, Widget *      )> = nullptr,
                std::function<void          (      Widget *, Widget *, bool)> = nullptr,

                Widget * = nullptr,
                bool     = false);

    public:
        void append(Widget *, bool);

    public:
        const Widget *getter(        ) const;
        void          setter(Widget *);

    private:
        void setButtonOff (TrigfxButton *);
        void setButtonDown(TrigfxButton *);
};
