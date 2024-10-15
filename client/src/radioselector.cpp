#include "radioselector.hpp"

RadioSelector::RadioSelector(Widget::VarDir argDir,

        Widget::VarOffset argX,
        Widget::VarOffset argY,

        std::initializer_list<std::tuple<Widget *, bool>> argWidgetList,

        int argGap,
        int argItemSpace,

        Widget * argParent,
        bool     argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          {},
          {},
          {},

          argParent,
          argAutoDelete,
      }

    , m_gap(std::max<int>(0, argGap))
    , m_itemSpace(std::max<int>(0, argItemSpace))
{
    for(auto [widget, autoDelete]: argWidgetList){
        append(widget, autoDelete);
    }
}

void RadioSelector::append(Widget *widget, bool autoDelete)
{
    auto button = new TritexButton
    {
        DIR_UPLEFT, // ignored
        0,
        0,

        {
            0X00000370,
            0X00000370,
            0X00000371,
        },

        {
            SYS_U32NIL,
            SYS_U32NIL,
            0X01020000 + 105,
        },

        nullptr,
        nullptr,
        nullptr,

        0,
        0,
        0,
        0,

        false,
        true,

        this,
        true,
    };

    const auto startX = 0;
    const auto startY = (hasChild() ? (h() + m_itemSpace) : 0) + std::max<int>(button->h(), widget->h()) / 2;

    addChild(DIR_LEFT, startX                      , startY, button, true);
    addChild(DIR_LEFT, startX + button->w() + m_gap, startY, widget, autoDelete);
}
