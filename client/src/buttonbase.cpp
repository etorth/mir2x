#include <functional>
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "buttonbase.hpp"
#include "sdldevice.hpp"
#include "soundeffectdb.hpp"

extern SDLDevice *g_sdlDevice;
extern SoundEffectDB *g_seffDB;

ButtonBase::ButtonBase(ButtonBase::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::move(args.w),
          .h = std::move(args.h),

          .attrs
          {
              .type
              {
                  .setSize  = false,
                  .addChild = false,
              },
              .inst = std::move(args.attrs),
          },
          .parent = std::move(args.parent),
      }}

    , m_onClickDone(args.onClickDone)
    , m_radioMode  (args.radioMode  )

    , m_seff(std::move(args.seff))
    , m_offset
      {
          {0               , 0               },
          {args.offXOnOver , args.offYOnOver },
          {args.offXOnClick, args.offYOnClick},
      }

    , m_onOverIn (std::move(args.onOverIn ))
    , m_onOverOut(std::move(args.onOverOut))
    , m_onClick  (std::move(args.onClick  ))
    , m_onTrigger(std::move(args.onTrigger))
{}

bool ButtonBase::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

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
                if(m.in(event.button.x, event.button.y)){
                    switch(getState()){
                        case BEVENT_OFF:
                            {
                                setState(BEVENT_ON);
                                setFocus(true);

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
                                    setFocus(true);

                                    onClick(true, event.button.clicks);
                                    if(m_onClickDone){
                                        onTrigger(event.button.clicks);
                                    }
                                }
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    return true;
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
                if(m.in(event.button.x, event.button.y)){
                    switch(getState()){
                        case BEVENT_OFF:
                            {
                                setState(BEVENT_DOWN);
                                setFocus(true);

                                onBadEvent();
                                break;
                            }
                        case BEVENT_ON:
                            {
                                setState(BEVENT_DOWN);
                                setFocus(true);

                                onClick(false, event.button.clicks);
                                if(!m_onClickDone){
                                    onTrigger(event.button.clicks);
                                }
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    return true;
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
                if(m.in(event.motion.x, event.motion.y)){
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
                    return true;
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
    Button::evalOverCBFunc(m_onOverIn, this);
    if(m_seff.onOverIn.has_value()){
        g_sdlDevice->playSoundEffect(g_seffDB->retrieve((m_seff.onOverIn.value())));
    }
}

void ButtonBase::onOverOut()
{
    Button::evalOverCBFunc(m_onOverOut, this);
    if(m_seff.onOverOut.has_value()){
        g_sdlDevice->playSoundEffect(g_seffDB->retrieve((m_seff.onOverOut.value())));
    }
}

void ButtonBase::onClick(bool clickDone, int clickCount)
{
    Button::evalClickCBFunc(m_onClick, this, clickDone, clickCount);
    if(clickDone){
        // pressed button released
    }
    else{
        if(m_seff.onClick.has_value()){
            g_sdlDevice->playSoundEffect(g_seffDB->retrieve((m_seff.onClick.value())));
        }
    }
}

void ButtonBase::onTrigger(int clickCount)
{
    Button::evalTriggerCBFunc(m_onTrigger, this, clickCount);
}

void ButtonBase::onBadEvent()
{
}
