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
            if(m_onOverOut){
                m_onOverOut();
            }
        }
        return focusConsume(this, false);
    }

    switch(event.type){
        case SDL_MOUSEBUTTONUP:
            {
                if(in(event.button.x, event.button.y)){
                    if(getState() != BEVENT_ON){
                        setState(BEVENT_ON);
                        if(m_onClickDone && m_onClick){
                            m_onClick();
                        }
                    }
                    return focusConsume(this, true);
                }
                else{
                    if(getState() != BEVENT_OFF){
                        setState(BEVENT_OFF);
                        if(m_onOverOut){
                            m_onOverOut();
                        }
                    }
                    return focusConsume(this, false);
                }
            }

        case SDL_MOUSEBUTTONDOWN:
            {
                if(in(event.button.x, event.button.y)){
                    if(getState() != BEVENT_DOWN){
                        setState(BEVENT_DOWN);
                        if(!m_onClickDone && m_onClick){
                            m_onClick();
                        }
                    }
                    return focusConsume(this, true);
                }
                else{
                    if(getState() != BEVENT_OFF){
                        setState(BEVENT_OFF);
                        if(m_onOverOut){
                            m_onOverOut();
                        }
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
                                if(m_onOverIn){
                                    m_onOverIn();
                                }
                                break;
                            }
                        case BEVENT_DOWN:
                            {
                                if(event.motion.state & SDL_BUTTON_LMASK){
                                    // hold the button and moving
                                    // trigger nothing
                                }
                                else{
                                    setState(BEVENT_ON);
                                    if(m_onOverIn){
                                        m_onOverIn();
                                    }
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
                        if(m_onOverOut){
                            m_onOverOut();
                        }
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
