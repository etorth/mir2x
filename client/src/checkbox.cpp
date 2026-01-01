#include "pngtexdb.hpp"
#include "checkbox.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

bool CheckBox::evalBoolGetter(const CheckBox::BoolGetter &getter, const Widget *widget)
{
    return std::visit(VarDispatcher
    {
        [widget](const std::function<bool(              )> &func){ return func ? func(      ) : dynamic_cast<const CheckBox *>(widget)->rawGetter(); },
        [widget](const std::function<bool(const Widget *)> &func){ return func ? func(widget) : dynamic_cast<const CheckBox *>(widget)->rawGetter(); },
        [widget](const auto                                &    ){ return                       dynamic_cast<const CheckBox *>(widget)->rawGetter(); },
    },
    getter);
}

void CheckBox::evalBoolSetter(CheckBox::BoolSetter &setter, Widget *widget, bool value)
{
    std::visit(VarDispatcher
    {
        [value, widget](std::function<void(          bool)> &func){ func ? func(        value) : dynamic_cast<CheckBox *>(widget)->rawSetter(value); },
        [value, widget](std::function<void(Widget *, bool)> &func){ func ? func(widget, value) : dynamic_cast<CheckBox *>(widget)->rawSetter(value); },
        [value, widget](auto                                &    ){                              dynamic_cast<CheckBox *>(widget)->rawSetter(value); },
    },
    setter);
}

void CheckBox::evalTriggerFunc(CheckBox::TriggerFunc &trigger, Widget *widget, bool value)
{
    std::visit(VarDispatcher
    {
        [value        ](std::function<void(          bool)> &func){ if(func){ func(        value); }},
        [value, widget](std::function<void(Widget *, bool)> &func){ if(func){ func(widget, value); }},

        [](auto &){},
    },
    trigger);
}

bool CheckBox::hasBoolGetter(const CheckBox::BoolGetter &getter)
{
    return std::visit(VarDispatcher
    {
        [](const std::function<bool(              )> &func){ return !!func;  },
        [](const std::function<bool(const Widget *)> &func){ return !!func;  },
        [](const auto &){ return false; },
    },
    getter);
}

bool CheckBox::hasBoolSetter(const CheckBox::BoolSetter &setter)
{
    return std::visit(VarDispatcher
    {
        [](const std::function<void(          bool)> &func){ return !!func;  },
        [](const std::function<void(Widget *, bool)> &func){ return !!func;  },
        [](const auto &){ return false; },
    },
    setter);
}

bool CheckBox::hasTriggerFunc(const CheckBox::TriggerFunc &trigger)
{
    return std::visit(VarDispatcher
    {
        [](const std::function<void(          bool)> &func){ return !!func;  },
        [](const std::function<void(Widget *, bool)> &func){ return !!func;  },
        [](const auto &){ return false; },
    },
    trigger);
}

CheckBox::CheckBox(CheckBox::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .parent = std::move(args.parent),
      }}

    , m_color(std::move(args.color))

    , m_valGetter  (std::move(args.getter  ))
    , m_valSetter  (std::move(args.setter  ))
    , m_valOnChange(std::move(args.onChange))

    , m_img
      {{
          .dir = DIR_NONE,

          .x = [this]{ return w() / 2; },
          .y = [this]{ return h() / 2; },

          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000480); },
          .parent{this},
      }}

    , m_box
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](int drawDstX, int drawDstY)
          {
              // +----1----+
              // |        ||
              // 4        52
              // |        ||
              // |----6---+|
              // +----3----+

              const auto  solidColor = Widget::evalU32(m_color, this);
              const auto shadowColor = colorf::maskRGB(solidColor) + colorf::A_SHF(colorf::A(solidColor) / 2);

              g_sdlDevice->drawLine( solidColor, drawDstX +       0, drawDstY +       0, drawDstX + w() - 1, drawDstY +       0); // 1
              g_sdlDevice->drawLine( solidColor, drawDstX + w() - 1, drawDstY +       0, drawDstX + w() - 1, drawDstY + h() - 1); // 2
              g_sdlDevice->drawLine( solidColor, drawDstX +       0, drawDstY + h() - 1, drawDstX + w() - 1, drawDstY + h() - 1); // 3
              g_sdlDevice->drawLine( solidColor, drawDstX +       0, drawDstY +       0, drawDstX +       0, drawDstY + h() - 1); // 4
              g_sdlDevice->drawLine(shadowColor, drawDstX + w() - 2, drawDstY +       0, drawDstX + w() - 2, drawDstY + h() - 2); // 5
              g_sdlDevice->drawLine(shadowColor, drawDstX +       0, drawDstY + h() - 2, drawDstX + w() - 2, drawDstY + h() - 2); // 6
          },

          .parent{this},
      }}
{
    m_img.setShow([this]{ return getter(); });
    setSize([argW = std::move(args.w), this]{ return Widget::evalSizeOpt(argW, this, [this]{ return std::max<int>({m_img.w(), m_img.h(), 16}); }); },
            [argH = std::move(args.h), this]{ return Widget::evalSizeOpt(argH, this, [this]{ return std::max<int>({m_img.w(), m_img.h(), 16}); }); });
}

bool CheckBox::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return consumeFocus(false);
    }

    if(!valid){
        return consumeFocus(false);
    }

    switch(event.type){
        case SDL_MOUSEBUTTONUP:
            {
                return consumeFocus(m.in(event.button.x, event.button.y));
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(m.in(event.button.x, event.button.y)){
                    setFocus(true);
                    toggle();
                    return true;
                }
                else{
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEMOTION:
            {
                return consumeFocus(m.in(event.motion.x, event.motion.y));
            }
        case SDL_KEYUP:
            {
                return consumeFocus(focus());
            }
        case SDL_KEYDOWN:
            {
                if(focus()){
                    switch(event.key.keysym.sym){
                        case SDLK_SPACE:
                        case SDLK_RETURN:
                            {
                                setFocus(true);
                                toggle();
                                return true;
                            }
                        default:
                            {
                                return true;
                            }
                    }
                }
                else{
                    return false;
                }
            }
        default:
            {
                return false;
            }
    }
}

void CheckBox::toggle()
{
    const bool value = !getter();
    setter(value);

    CheckBox::evalTriggerFunc(m_valOnChange, this, value);
}

bool CheckBox::getter() const
{
    return CheckBox::evalBoolGetter(m_valGetter, this);
}

void CheckBox::setter(bool value)
{
    CheckBox::evalBoolSetter(m_valSetter, this, value);
}

bool CheckBox::rawGetter() const
{
    return m_innVal;
}

void CheckBox::rawSetter(bool value)
{
    m_innVal = value;
}
