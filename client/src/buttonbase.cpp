#include <functional>
#include "sdldevice.hpp"
#include "buttonbase.hpp"
#include "sdldevice.hpp"
#include "soundeffectdb.hpp"

extern SDLDevice *g_sdlDevice;
extern SoundEffectDB *g_seffDB;

ButtonBase::ButtonBase(
        dir8_t argDir,
        int argX,
        int argY,
        int argW,
        int argH,

        std::function<void(ButtonBase *)> fnOnOverIn,
        std::function<void(ButtonBase *)> fnOnOverOut,
        std::function<void(ButtonBase *)> fnOnClick,

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
          argDir,
          argX,
          argY,
          argW,
          argH,

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

    , m_onOverIn (std::move(fnOnOverIn))
    , m_onOverOut(std::move(fnOnOverOut))
    , m_onClick  (std::move(fnOnClick))
{}

bool ButtonBase::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        if(getState() != BEVENT_OFF){
            setState(BEVENT_OFF);
            onOverOut();
        }
        return consumeFocus(false);
    }

    if(!active()){
        if(getState() != BEVENT_OFF){
            setState(BEVENT_OFF);
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
                                setState(BEVENT_ON);
                                if(m_onClickDone){
                                    onClick();
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
                                if(!m_onClickDone){
                                    onClick();
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

void ButtonBase::onClick()
{
    if(m_onClick){
        m_onClick(this);
    }

    if(m_seffID[2].has_value()){
        g_sdlDevice->playSoundEffect(g_seffDB->retrieve((m_seffID[2].value())));
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

void ButtonBase::onBadEvent()
{
}
