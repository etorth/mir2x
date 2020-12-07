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
        if(m_state != BEVENT_OFF){
            if(m_onOverOut){
                m_onOverOut();
            }
        }

        m_state = BEVENT_OFF;
        return focusConsumer(this, false);
    }

    switch(event.type){
        case SDL_MOUSEBUTTONUP:
            {
                if(in(event.button.x, event.button.y)){
                    if(m_onClickDone && m_onClick){
                        m_onClick();
                    }

                    m_state = BEVENT_ON;
                    return focusConsumer(this, true);
                }
                else{
                    m_state = BEVENT_OFF;
                    return focusConsumer(this, false);
                }
            }

        case SDL_MOUSEBUTTONDOWN:
            {
                if(in(event.button.x, event.button.y)){
                    if(!m_onClickDone && m_onClick){
                        m_onClick();
                    }

                    m_state = BEVENT_DOWN;
                    return focusConsumer(this, true);
                }
                else{
                    m_state = BEVENT_OFF;
                    return focusConsumer(this, false);
                }
            }
        case SDL_MOUSEMOTION:
            {
                if(in(event.motion.x, event.motion.y)){
                    if(m_state != BEVENT_ON){
                        if(m_onOverIn){
                            m_onOverIn();
                        }
                    }

                    m_state = BEVENT_ON;
                    return focusConsumer(this, true);
                }
                else{
                    if(m_state != BEVENT_OFF){
                        if(m_onOverOut){
                            m_onOverOut();
                        }
                    }

                    m_state = BEVENT_OFF;
                    return focusConsumer(this, false);
                }
            }
        default:
            {
                return focusConsumer(this, false);
            }
    }
}
