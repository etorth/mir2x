#include "pngtexdb.hpp"
#include "trigfxbutton.hpp"
#include "radioselector.hpp"

extern PNGTexDB *g_progUseDB;

RadioSelector::RadioSelector(Widget::VarDir argDir,

        Widget::VarInt argX,
        Widget::VarInt argY,

        int argGap,
        int argItemSpace,

        std::initializer_list<std::tuple<Widget *, bool>> argWidgetList,

        std::function<const Widget *(const Widget *                )> argValGetter,
        std::function<void          (      Widget *, Widget *      )> argValSetter,
        std::function<void          (      Widget *, Widget *, bool)> argValOnChange,

        Widget * argParent,
        bool     argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),
          .w = std::nullopt,
          .h = std::nullopt,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_gap(std::max<int>(0, argGap))
    , m_itemSpace(std::max<int>(0, argItemSpace))

    , m_valGetter(std::move(argValGetter))
    , m_valSetter(std::move(argValSetter))
    , m_valOnChange(std::move(argValOnChange))

    , m_imgOff  {{.w=16, .h=16, .texLoadFunc=[](const Widget *){ return g_progUseDB->retrieve(0X00000370); }, .modColor=colorf::WHITE_A255}}
    , m_imgOn   {{.w=16, .h=16, .texLoadFunc=[](const Widget *){ return g_progUseDB->retrieve(0X00000370); }, .modColor=colorf::  RED_A255}}
    , m_imgDown {{.w=16, .h=16, .texLoadFunc=[](const Widget *){ return g_progUseDB->retrieve(0X00000371); }, .modColor=colorf::WHITE_A255}}

{
    for(auto [widget, autoDelete]: argWidgetList){
        append(widget, autoDelete);
    }
}

void RadioSelector::append(Widget *widget, bool autoDelete)
{
    auto button = new RadioSelector::InternalRadioButton
    {{
        .gfxList
        {
            &m_imgOff,
            &m_imgOn,
            &m_imgDown,
        },

        .onTrigger = [this](Widget *selfButton, int)
        {
            setter(getRadioWidget(selfButton));
            foreachRadioButton([selfButton, this](Widget *button)
            {
                if(selfButton != button){
                    dynamic_cast<TrigfxButton *>(button)->setOff();
                }

                if(m_valOnChange){
                    m_valOnChange(this, getRadioWidget(button), selfButton == button);
                }
            });
        },

        .onClickDone = false,
        .radioMode = true,

        .attrs
        {
            .inst
            {
                .data = std::make_any<Widget *>(widget),
            },
        },
        .parent{this},
    }};

    if(getter() == widget){
        button->setDown();
    }
    else{
        button->setOff();
    }

    const auto startX = 0;
    const auto startY = (hasChild() ? (h() + m_itemSpace) : 0) + std::max<int>(button->h(), widget->h()) / 2;

    addChildAt(button, DIR_LEFT, startX                      , startY, true);
    addChildAt(widget, DIR_LEFT, startX + button->w() + m_gap, startY, autoDelete);
}

const Widget *RadioSelector::getter() const
{
    if(m_valGetter){
        return m_valGetter(this);
    }
    else{
        return m_selected;
    }
}

void RadioSelector::setter(Widget *selected)
{
    if(!m_valGetter){
        m_selected = selected;
    }

    if(m_valSetter){
        m_valSetter(this, selected);
    }
}
