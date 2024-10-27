#include "pngtexdb.hpp"
#include "trigfxbutton.hpp"
#include "radioselector.hpp"

extern PNGTexDB *g_progUseDB;

RadioSelector::RadioSelector(Widget::VarDir argDir,

        Widget::VarOff argX,
        Widget::VarOff argY,

        int argGap,
        int argItemSpace,

        std::initializer_list<std::tuple<Widget *, bool>> argWidgetList,

        std::function<const Widget *(const Widget *                )> argValGetter,
        std::function<void          (      Widget *, Widget *      )> argValSetter,
        std::function<void          (      Widget *, Widget *, bool)> argValOnChange,

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

    , m_valGetter(std::move(argValGetter))
    , m_valSetter(std::move(argValSetter))
    , m_valOnChange(std::move(argValOnChange))

    , m_imgOff  {DIR_UPLEFT, 0, 0, 16, 16, [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000370); }, false, false, 0, colorf::WHITE + colorf::A_SHF(0XFF)}
    , m_imgOn   {DIR_UPLEFT, 0, 0, 16, 16, [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000370); }, false, false, 0, colorf::RED   + colorf::A_SHF(0XFF)}
    , m_imgDown {DIR_UPLEFT, 0, 0, 16, 16, [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000371); }, false, false, 0, colorf::WHITE + colorf::A_SHF(0XFF)}

{
    for(auto [widget, autoDelete]: argWidgetList){
        append(widget, autoDelete);
    }
}

void RadioSelector::append(Widget *widget, bool autoDelete)
{
    auto button = new RadioSelector::InternalRadioButton
    {
        DIR_UPLEFT, // ignored
        0,
        0,

        {
            &m_imgOff,
            &m_imgOn,
            &m_imgDown,
        },

        {
            std::nullopt,
            std::nullopt,
            0X01020000 + 105,
        },

        nullptr,
        nullptr,
        [this](Widget *selfButton)
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

        0,
        0,
        0,
        0,

        false,
        true,

        this,
        true,
    };

    button->setData(widget);
    if(getter() == widget){
        button->setDown();
    }
    else{
        button->setOff();
    }

    const auto startX = 0;
    const auto startY = (hasChild() ? (h() + m_itemSpace) : 0) + std::max<int>(button->h(), widget->h()) / 2;

    addChild(button, DIR_LEFT, startX                      , startY, true);
    addChild(widget, DIR_LEFT, startX + button->w() + m_gap, startY, autoDelete);
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
