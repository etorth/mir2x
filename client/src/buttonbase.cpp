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
        m_State = BUTTON_OFF;
        return false;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONUP:
            {
                if(in(event.button.x, event.button.y)){
                    if(m_OnClickDone){
                        m_OnClick();
                    }

                    m_State = BUTTON_OVER;
                    return true;
                }
                else{
                    m_State = BUTTON_OFF;
                    return false;
                }
            }

        case SDL_MOUSEBUTTONDOWN:
            {
                if(in(event.button.x, event.button.y)){
                    if(!m_OnClickDone){
                        m_OnClick();
                    }

                    m_State = BUTTON_PRESSED;
                    return true;
                }
                else{
                    m_State = BUTTON_OFF;
                    return false;
                }
            }
        case SDL_MOUSEMOTION:
            {
                if(in(event.motion.x, event.motion.y)){
                    if(m_State != 2){
                        m_State = BUTTON_OVER;
                    }
                    return true;
                }
                else{
                    m_State = BUTTON_OFF;
                    return false;
                }
            }
        default:
            {
                return false;
            }
    }
}
