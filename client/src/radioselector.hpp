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
        class InternalRadioButton: public TrigfxButton
        {
            using TrigfxButton::TrigfxButton;
        };

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

    public:
        auto foreachRadioButton(std::invocable<Widget *> auto f)
        {
            constexpr bool hasBoolResult = std::is_same_v<std::invoke_result_t<decltype(f), Widget *>, bool>;
            if constexpr (hasBoolResult){
                return foreachChild([&f](Widget *child, bool)
                {
                    if(dynamic_cast<RadioSelector::InternalRadioButton *>(child)){
                        if(f(child)){
                            return true;
                        }
                    }
                    return false;
                });
            }
            else{
                foreachChild([&f](Widget *child, bool)
                {
                    if(dynamic_cast<RadioSelector::InternalRadioButton *>(child)){
                        f(child);
                    }
                });
            }
        }

        auto foreachRadioWidget(std::invocable<Widget *> auto f)
        {
            constexpr bool hasBoolResult = std::is_same_v<std::invoke_result_t<decltype(f), Widget *>, bool>;
            if constexpr (hasBoolResult){
                return foreachRadioButton([&f](Widget *button)
                {
                    return f(getRadioWidget(button));
                });
            }
            else{
                foreachRadioButton([&f](Widget *button)
                {
                    f(getRadioWidget(button));
                });
            }
        }

    public:
        static Widget *getRadioWidget(Widget *button)
        {
            fflassert(button);
            fflassert(dynamic_cast<RadioSelector::InternalRadioButton *>(button));
            return std::any_cast<Widget *>(button->data());
        }
};
