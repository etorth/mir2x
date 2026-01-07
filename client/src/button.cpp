#include "button.hpp"
#include "protocoldef.hpp"

void Button::evalOverCBFunc(const Button::OverCBFunc &func, Widget *widget)
{
    std::visit(VarDispatcher
    {
        [      ](const std::function<void(        )> &f){ if(f){f(      );} },
        [widget](const std::function<void(Widget *)> &f){ if(f){f(widget);} },

        [](auto &){},

    }, func);
}

void Button::evalClickCBFunc(const Button::ClickCBFunc &func, Widget *widget, bool clickDone, int clickCount)
{
    std::visit(VarDispatcher
    {
        [        clickDone, clickCount](const std::function<void(          bool, int)> &f){ if(f){f(        clickDone, clickCount);} },
        [widget, clickDone, clickCount](const std::function<void(Widget *, bool, int)> &f){ if(f){f(widget, clickDone, clickCount);} },

        [](auto &){},

    }, func);
}

void Button::evalTriggerCBFunc(const Button::TriggerCBFunc &func, Widget *widget, int clickCount)
{
    std::visit(VarDispatcher
    {
        [        clickCount](const std::function<void(          int)> &f){ if(f){f(        clickCount);} },
        [widget, clickCount](const std::function<void(Widget *, int)> &f){ if(f){f(widget, clickCount);} },

        [](auto &){},

    }, func);
}
