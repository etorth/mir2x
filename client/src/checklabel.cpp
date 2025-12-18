#include "widget.hpp"
#include "sdldevice.hpp"
#include "checklabel.hpp"

CheckLabel::CheckLabel(
        dir8_t argDir,
        int argX,
        int argY,

        bool argBoxFirst,
        int argGap,

        uint32_t argBoxColor,
        int argBoxW,
        int argBoxH,

        std::function<bool(const Widget *      )> argValGetter,
        std::function<void(      Widget *, bool)> argValSetter,
        std::function<void(      Widget *, bool)> argValOnChange,

        const char8_t *argLabel,
        uint8_t  argFont,
        uint8_t  argFontSize,
        uint8_t  argFontStyle,
        uint32_t argFontColor,

        Widget *argParent,
        bool argAutoDelete)

    : Widget
      ({
          .dir = argDir,
          .x = argX,
          .y = argY,
          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      })

    , m_checkBoxColor(argBoxColor)
    , m_labelBoardColor(argFontColor)

    , m_checkBox
      {
          DIR_UPLEFT,
          0,
          0,
          argBoxW,
          argBoxH,

          argBoxColor,

          [argValGetter = std::move(argValGetter), this]() -> std::function<bool(const Widget *)>
          {
              if(!argValGetter){
                  return nullptr;
              }

              return [argValGetter = std::move(argValGetter), this](const Widget *) -> bool
              {
                  return argValGetter(this);
              };
          }(),

          [argValSetter = std::move(argValSetter), this]() -> std::function<void(Widget *, bool)>
          {
              if(!argValSetter){
                  return nullptr;
              }

              return [argValSetter = std::move(argValSetter), this](Widget *, bool value)
              {
                  argValSetter(this, value);
              };
          }(),


          [argValOnChange = std::move(argValOnChange), this]() -> std::function<void(Widget *, bool)>
          {
              if(!argValOnChange){
                  return nullptr;
              }

              return [argValOnChange = std::move(argValOnChange), this](Widget *, bool value)
              {
                  argValOnChange(this, value);
              };
          }(),

          this,
          false,
      }

    , m_labelBoard
      {{
          .label = argLabel,
          .font
          {
              .id = argFont,
              .size = argFontSize,
              .style = argFontStyle,
              .color = std::move(argFontColor),
          },
          .parent{this},
      }}
{
    setW(m_checkBox.w() + std::max<int>(argGap, 0) + m_labelBoard.w());
    setH(std::max<int>(m_checkBox.h(), m_labelBoard.h()));

    if(argBoxFirst){
        m_checkBox  .moveAt(DIR_LEFT ,       0, h() / 2);
        m_labelBoard.moveAt(DIR_RIGHT, w() - 1, h() / 2);
    }
    else{
        m_labelBoard.moveAt(DIR_LEFT ,       0, h() / 2);
        m_checkBox  .moveAt(DIR_RIGHT, w() - 1, h() / 2);
    }
}

bool CheckLabel::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!show()){
        return false;
    }

    if(!m.calibrate(this)){
        return false;
    }

    const auto fnOnColor = [this](bool on)
    {
        if(on){
            m_checkBox.setColor(colorf::modRGBA(m_checkBoxColor, colorf::RGBA(0XFF, 0, 0, 0XFF)));
            m_labelBoard.setFontColor(colorf::modRGBA(m_labelBoardColor, colorf::RGBA(0XFF, 0, 0, 0XFF)));
        }
        else{
            m_checkBox.setColor(m_checkBoxColor);
            m_labelBoard.setFontColor(m_labelBoardColor);
        }
    };

    if(!valid){
        fnOnColor(false);
        return consumeFocus(false);
    }


    switch(event.type){
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                fnOnColor(m.in(SDLDeviceHelper::getEventPLoc(event).value()));
                break;
            }
        default:
            {
                break;
            }
    }

    if(m_checkBox.processEvent(event, valid, m.create(m_checkBox.roi()))){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONUP:
            {
                if(m.in(event.button.x, event.button.y)){
                    return consumeFocus(true, &m_checkBox);
                }
                else{
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(m.in(event.button.x, event.button.y)){
                    m_checkBox.toggle();
                    return consumeFocus(true, &m_checkBox);
                }
                else{
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEMOTION:
            {
                if(m.in(event.motion.x, event.motion.y)){
                    return consumeFocus(true, &m_checkBox);
                }
                else{
                    return consumeFocus(false);
                }
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
                                m_checkBox.toggle();
                                return consumeFocus(true, &m_checkBox);
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

void CheckLabel::setFocus(bool argFocus)
{
    Widget::setFocus(false);
    if(argFocus){
        m_checkBox.setFocus(true);
    }
}
