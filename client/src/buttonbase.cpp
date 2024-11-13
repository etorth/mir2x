#include <functional>
#include "sdldevice.hpp"
#include "buttonbase.hpp"
#include "sdldevice.hpp"
#include "soundeffectdb.hpp"

extern SDLDevice *g_sdlDevice;
extern SoundEffectDB *g_seffDB;

ButtonBase::ButtonBase(
        Widget::VarDir  argDir,
        Widget::VarOff  argX,
        Widget::VarOff  argY,
        Widget::VarSize argW,
        Widget::VarSize argH,

        std::function<void(Widget *      )> fnOnOverIn,
        std::function<void(Widget *      )> fnOnOverOut,
        std::function<void(Widget *, bool)> fnOnClick,
        std::function<void(Widget *      )> fnOnTrigger,

        std::optional<uint32_t> seffIDOnOverIn,
        std::optional<uint32_t> seffIDOnOverOut,
        std::optional<uint32_t> seffIDOnClick,

        int offXOnOver,
        int offYOnOver,
        int offXOnClick,
        int offYOnClick,

        bool onClickDone,
        bool radioMode,

        Widget *widgetPtr,
        bool    autoFree)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          std::move(argW),
          std::move(argH),

          {},

          widgetPtr,
          autoFree,
      }

    , m_onClickDone(onClickDone)
    , m_radioMode(radioMode)

    , m_seffID
      {
          seffIDOnOverIn,
          seffIDOnOverOut,
          seffIDOnClick,
      }

    , m_offset
      {
          {0            , 0          },
          {offXOnOver   , offYOnOver },
          {offXOnClick  , offYOnClick},
      }

    , m_onOverIn (std::move(fnOnOverIn ))
    , m_onOverOut(std::move(fnOnOverOut))
    , m_onClick  (std::move(fnOnClick  ))
    , m_onTrigger(std::move(fnOnTrigger))
{}

bool ButtonBase::processEventDefault(const SDL_Event &event, bool valid)
{
    if(!valid){
        if(m_radioMode){
            if(getState() == BEVENT_ON){
                setState(BEVENT_OFF);
                onOverOut();
            }
        }
        else{
            if(getState() != BEVENT_OFF){
                setState(BEVENT_OFF);
                onOverOut();
            }
        }
        return consumeFocus(false);
    }

    if(!active()){
        if(m_radioMode){
            if(getState() == BEVENT_ON){
                setState(BEVENT_OFF);
            }
        }
        else{
            if(getState() != BEVENT_OFF){
                setState(BEVENT_OFF);
            }
        }
        return consumeFocus(false);
    }

    switch(event.type){
        case SDL_MOUSEBUTTONUP:
            {
                if(in(event.button.x, event.button.y)){
                    switch(getState()){
                        case BEVENT_OFF:
                            {
                                setState(BEVENT_ON);
                                onBadEvent();
                                break;
                            }
                        case BEVENT_DOWN:
                            {
                                if(m_radioMode){
                                    // keep pressed
                                }
                                else{
                                    setState(BEVENT_ON);
                                    onClick(true);
                                    if(m_onClickDone){
                                        onTrigger();
                                    }
                                }
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    return consumeFocus(true);
                }
                else if(m_radioMode){
                    return consumeFocus(false);
                }
                else{
                    if(getState() != BEVENT_OFF){
                        setState(BEVENT_OFF);
                        onOverOut();
                    }
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(in(event.button.x, event.button.y)){
                    switch(getState()){
                        case BEVENT_OFF:
                            {
                                setState(BEVENT_DOWN);
                                onBadEvent();
                                break;
                            }
                        case BEVENT_ON:
                            {
                                setState(BEVENT_DOWN);
                                onClick(false);
                                if(!m_onClickDone){
                                    onTrigger();
                                }
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    return consumeFocus(true);
                }
                else if(m_radioMode){
                    return consumeFocus(false);
                }
                else{
                    if(getState() != BEVENT_OFF){
                        setState(BEVENT_OFF);
                        onOverOut();
                    }
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEMOTION:
            {
                if(in(event.motion.x, event.motion.y)){
                    switch(getState()){
                        case BEVENT_OFF:
                            {
                                setState(BEVENT_ON);
                                onOverIn();
                                break;
                            }
                        case BEVENT_DOWN:
                            {
                                if(event.motion.state & SDL_BUTTON_LMASK){
                                    // hold the button and moving
                                    // don't trigger
                                }
                                else if(m_radioMode){
                                    // keep pressed
                                }
                                else{
                                    setState(BEVENT_ON);
                                    onBadEvent();
                                }
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    return consumeFocus(true);
                }
                else if(m_radioMode){
                    if(getState() == BEVENT_ON){
                        setState(BEVENT_OFF);
                        onOverOut();
                    }
                    return consumeFocus(false);
                }
                else{
                    if(getState() != BEVENT_OFF){
                        setState(BEVENT_OFF);
                        onOverOut();
                    }
                    return consumeFocus(false);
                }
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

void ButtonBase::onOverIn()
{
    if(m_onOverIn){
        m_onOverIn(this);
    }

    if(m_seffID[0].has_value()){
        g_sdlDevice->playSoundEffect(g_seffDB->retrieve((m_seffID[0].value())));
    }
}

void ButtonBase::onOverOut()
{
    if(m_onOverOut){
        m_onOverOut(this);
    }

    if(m_seffID[1].has_value()){
        g_sdlDevice->playSoundEffect(g_seffDB->retrieve((m_seffID[1].value())));
    }
}

void ButtonBase::onClick(bool clickDone)
{
    if(m_onClick){
        m_onClick(this, clickDone);
    }

    if(m_seffID[2].has_value()){
        g_sdlDevice->playSoundEffect(g_seffDB->retrieve((m_seffID[2].value())));
    }
}

void ButtonBase::onTrigger()
{
    if(m_onTrigger){
        m_onTrigger(this);
    }
}

void ButtonBase::onBadEvent()
{
}
