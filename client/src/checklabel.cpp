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
      {
          argDir,
          argX,
          argY,
          0,
          0,

          {},

          argParent,
          argAutoDelete,
      }

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
      {
          DIR_UPLEFT,
          0,
          0,

          argLabel,
          argFont,
          argFontSize,
          argFontStyle,
          argFontColor,

          this,
          false,
      }
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

bool CheckLabel::processEvent(const SDL_Event &event, bool valid)
{
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

    if(!show()){
        return consumeFocus(false);
    }

    switch(event.type){
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                const auto [eventX, eventY] = SDLDeviceHelper::getEventPLoc(event).value();
                fnOnColor(in(eventX, eventY));
                break;
            }
        default:
            {
                break;
            }
    }

    if(m_checkBox.processEvent(event, valid)){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONUP:
            {
                if(in(event.button.x, event.button.y)){
                    return consumeFocus(true, &m_checkBox);
                }
                else{
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(in(event.button.x, event.button.y)){
                    m_checkBox.toggle();
                    return consumeFocus(true, &m_checkBox);
                }
                else{
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEMOTION:
            {
                if(in(event.motion.x, event.motion.y)){
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

void CheckLabel::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(!show()){
        return;
    }

    for(auto p:
    {
        static_cast<const Widget *>(&m_checkBox),
        static_cast<const Widget *>(&m_labelBoard),
    }){
        if(!p->show()){
            continue;
        }

        int srcXCrop = srcX;
        int srcYCrop = srcY;
        int dstXCrop = dstX;
        int dstYCrop = dstY;
        int srcWCrop = srcW;
        int srcHCrop = srcH;

        if(mathf::cropChildROI(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    w(),
                    h(),

                    p->dx(),
                    p->dy(),
                    p-> w(),
                    p-> h())){
            p->drawEx(dstXCrop, dstYCrop, srcXCrop, srcYCrop, srcWCrop, srcHCrop);
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
