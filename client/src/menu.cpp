#include "menu.hpp"

void Menu::evalClickCBFunc(const ClickCBFunc &cbFunc, Widget *widget)
{
    std::visit(VarDispatcher
    {
        [      ](const std::function<void(        )> &f){ if(f){ f(      ); }},
        [widget](const std::function<void(Widget *)> &f){ if(f){ f(widget); }},

        [](const auto &){},
    },

    cbFunc);
}
