#include "widget.hpp"
#include "sdldevice.hpp"
#include "checklabel.hpp"

CheckLabel::CheckLabel(CheckLabel::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .parent = std::move(args.parent),
      }}

    , m_box
      {{
          .w = std::move(args.box.w),
          .h = std::move(args.box.h),

          .color = [bcolor = std::move(args.box.color), this] -> uint32_t
          {
              if(const auto color = Widget::evalU32(bcolor, this); m_hoverColor){
                  return colorf::modRGBA(color, colorf::RGBA(0XFF, 0, 0, 0XFF));
              }
              else{
                  return color;
              }
          },

          .getter = [getter = std::move(args.getter), this] -> CheckLabel::BoolGetter
          {
              if(CheckLabel::hasBoolGetter(getter)){
                  return [getter = std::move(getter), this]
                  {
                      return CheckLabel::evalBoolGetter(getter, this);
                  };
              }
              else return nullptr;
          }(),

          .setter = [setter = std::move(args.setter), this] -> CheckLabel::BoolSetter
          {
              if(CheckLabel::hasBoolSetter(setter)){
                  return [setter = std::move(setter), this](bool value) mutable
                  {
                      CheckLabel::evalBoolSetter(setter, this, value);
                  };
                }
                else return nullptr;
          }(),

          .onChange = [trigger = std::move(args.onChange), this] -> CheckLabel::TriggerFunc
          {
              if(CheckLabel::hasTriggerFunc(trigger)){
                  return [trigger = std::move(trigger), this](bool value) mutable
                  {
                      CheckLabel::evalTriggerFunc(trigger, this, value);
                  };
              }
              else return nullptr;
          }(),

          .parent{this},
      }}

    , m_label
      {{
          .label = args.label.text,
          .font
          {
              .id = args.label.font.id,
              .size = args.label.font.size,
              .style = args.label.font.style,

              .color = [fcolor = std::move(args.label.font.color), this] -> uint32_t
              {
                  if(const auto color = Widget::evalU32(fcolor, this); m_hoverColor){
                      return colorf::modRGBA(color, colorf::RGBA(0XFF, 0, 0, 0XFF));
                  }
                  else{
                      return color;
                  }
              },
              .bgColor = std::move(args.label.font.bgColor),
          },
          .parent{this},
      }}
{
    setSize([gap = std::move(args.gap), this]
    {
        return m_box.w() + Widget::evalSizeOpt(gap, this, [this]{ return m_box.w() / 2; }) + m_label.w();
    },

    [this]
    {
        return std::max<int>(m_box.h(), m_label.h());
    });

    if(args.boxFirst){
        m_box  .moveAt(DIR_LEFT ,       0, h() / 2);
        m_label.moveAt(DIR_RIGHT, w() - 1, h() / 2);
    }
    else{
        m_label.moveAt(DIR_LEFT ,       0, h() / 2);
        m_box  .moveAt(DIR_RIGHT, w() - 1, h() / 2);
    }
}

bool CheckLabel::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        m_hoverColor = false;
        return consumeFocus(false);
    }


    switch(event.type){
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                m_hoverColor = m.in(SDLDeviceHelper::getEventPLoc(event).value());
                break;
            }
        default:
            {
                break;
            }
    }

    if(m_box.processEvent(event, valid, m.create(m_box.roi()))){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONUP:
            {
                if(m.in(event.button.x, event.button.y)){
                    return consumeFocus(true, &m_box);
                }
                else{
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(m.in(event.button.x, event.button.y)){
                    m_box.toggle();
                    return consumeFocus(true, &m_box);
                }
                else{
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEMOTION:
            {
                if(m.in(event.motion.x, event.motion.y)){
                    return consumeFocus(true, &m_box);
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
                                m_box.toggle();
                                return consumeFocus(true, &m_box);
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
