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

bool buttonBase::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        m_state = BUTTON_OFF;
        return false;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONUP:
            {
                if(in(event.button.x, event.button.y)){
                    if(m_onClickDone && m_onClick){
                        m_onClick();
                    }

                    m_state = BUTTON_OVER;
                    return true;
                }
                else{
                    m_state = BUTTON_OFF;
                    return false;
                }
            }

        case SDL_MOUSEBUTTONDOWN:
            {
                if(in(event.button.x, event.button.y)){
                    if(!m_onClickDone && m_onClick){
                        m_onClick();
                    }

                    m_state = BUTTON_PRESSED;
                    return true;
                }
                else{
                    m_state = BUTTON_OFF;
                    return false;
                }
            }
        case SDL_MOUSEMOTION:
            {
                if(in(event.motion.x, event.motion.y)){
                    if(m_state != 2){
                        if(m_onOver){
                            m_onOver();
                        }
                        m_state = BUTTON_OVER;
                    }
                    return true;
                }
                else{
                    m_state = BUTTON_OFF;
                    return false;
                }
            }
        default:
            {
                return false;
            }
    }
}
