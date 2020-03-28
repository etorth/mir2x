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

bool ButtonBase::processEvent(const SDL_Event &rstEvent, bool *bValid)
{
    if(bValid && !(*bValid)){
        return false;
    }

    switch(rstEvent.type){
        case SDL_MOUSEBUTTONUP:
            {
                if(In(rstEvent.button.x, rstEvent.button.y)){
                    if(m_OnClickDone){
                        m_OnClick();
                    }

                    m_State = BUTTON_OVER;
                    return true;
                }else{
                    m_State = BUTTON_OFF;
                    return false;
                }
                break;
            }

        case SDL_MOUSEBUTTONDOWN:
            {
                if(In(rstEvent.button.x, rstEvent.button.y)){
                    if(!m_OnClickDone){
                        m_OnClick();
                    }

                    m_State = BUTTON_PRESSED;
                    return true;
                }else{
                    m_State = BUTTON_OFF;
                    return false;
                }
                break;
            }
        case SDL_MOUSEMOTION:
            {
                if(In(rstEvent.motion.x, rstEvent.motion.y)){
                    if(m_State != 2){
                        m_State = BUTTON_OVER;
                    }
                    return true;
                }else{
                    m_State = BUTTON_OFF;
                    return false;
                }
                break;
            }
        default:
            {
                return false;
            }
    }
    return false;
}
