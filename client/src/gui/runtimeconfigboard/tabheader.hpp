#pragma once
#include <functional>
#include <any>
#include "widget.hpp"
#include "labelboard.hpp"
#include "trigfxbutton.hpp"

class TabHeader: public Widget
{
    private:
        LabelBoard m_label;
        TrigfxButton m_button;

    public:
        TabHeader(dir8_t,
                int,
                int,

                const char8_t *,
                std::function<void(Widget *, int)>,

                std::any,

                Widget * = nullptr,
                bool     = false);
};
