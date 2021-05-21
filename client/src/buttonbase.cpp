/*
 * =====================================================================================
 *
 *       Filename: buttonbase.cpp
 *        Created: 08/21/2015 04:12:57
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include <functional>
#include "sdldevice.hpp"
#include "buttonbase.hpp"

bool ButtonBase::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        if(getState() != BEVENT_OFF){
            setState(BEVENT_OFF);
            onOverOut();
        }
        return focusConsume(this, false);
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
                    return focusConsume(this, true);
                }
                else{
                    if(getState() != BEVENT_OFF){
                        setState(BEVENT_OFF);
                        onOverOut();
                    }
                    return focusConsume(this, false);
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
                    return focusConsume(this, true);
                }
                else{
                    if(getState() != BEVENT_OFF){
                        setState(BEVENT_OFF);
                        onOverOut();
                    }
                    return focusConsume(this, false);
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
                    return focusConsume(this, true);
                }
                else{
                    if(getState() != BEVENT_OFF){
                        setState(BEVENT_OFF);
                        onOverOut();
                    }
                    return focusConsume(this, false);
                }
            }
        default:
            {
                return focusConsume(this, false);
            }
    }
}

void ButtonBase::onClick()
{
    if(m_onClick){
        m_onClick();
    }
}

void ButtonBase::onOverIn()
{
    if(m_onOverIn){
        m_onOverIn();
    }
}

void ButtonBase::onOverOut()
{
    if(m_onOverOut){
        m_onOverOut();
    }
}

void ButtonBase::onBadEvent()
{
}
