#include "menu.hpp"
#include "stdf.hpp"

void Menu::evalClickCBFunc(const ClickCBFunc &cbFunc, Widget *widget)
{
    std::visit(stdf::VarDispatcher
    {
        [      ](const std::function<void(        )> &f){ if(f){ f(      ); }},
        [widget](const std::function<void(Widget *)> &f){ if(f){ f(widget); }},

        [](const auto &){},
    },

    cbFunc);
}
