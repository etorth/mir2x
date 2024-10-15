#include "pngtexdb.hpp"
#include "trigfxbutton.hpp"
#include "radioselector.hpp"

extern PNGTexDB *g_progUseDB;

RadioSelector::RadioSelector(Widget::VarDir argDir,

        Widget::VarOffset argX,
        Widget::VarOffset argY,

        int argGap,
        int argItemSpace,

        std::initializer_list<std::tuple<Widget *, bool>> argWidgetList,
        std::function<void(Widget *, bool)> argOnChange,

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

    , m_onChange(std::move(argOnChange))

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
    auto button = new TrigfxButton
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
        [this](ButtonBase *self)
        {
            foreachChild([self, this](Widget *child, bool)
            {
                if(auto buttonPtr = dynamic_cast<TrigfxButton *>(child)){
                    if(buttonPtr == self){
                        buttonPtr->setGfxList(
                        {
                            &m_imgDown,
                            &m_imgDown,
                            &m_imgDown,
                        });
                    }
                    else{
                        buttonPtr->setGfxList(
                        {
                            &m_imgOff,
                            &m_imgOn,
                            &m_imgDown,
                        });
                        buttonPtr->setOff();
                    }

                    if(m_onChange){
                        m_onChange(std::any_cast<Widget *>(buttonPtr->data()), buttonPtr == self);
                    }
                }
            });
        },

        0,
        0,
        0,
        0,

        false,

        this,
        true,
    };

    button->setData(widget);

    const auto startX = 0;
    const auto startY = (hasChild() ? (h() + m_itemSpace) : 0) + std::max<int>(button->h(), widget->h()) / 2;

    addChild(DIR_LEFT, startX                      , startY, button, true);
    addChild(DIR_LEFT, startX + button->w() + m_gap, startY, widget, autoDelete);
}
