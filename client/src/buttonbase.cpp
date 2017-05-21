/*
 * =====================================================================================
 *
 *       Filename: buttonbase.cpp
 *        Created: 08/21/2015 04:12:57
 *  Last Modified: 05/19/2017 18:33:06
 *
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

#include <algorithm>
#include <functional>

#include "log.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"
#include "buttonbase.hpp"

bool ButtonBase::ProcessEvent(const SDL_Event &rstEvent, bool *bValid)
{
    if(bValid && !(*bValid)){ return false; }

    switch(rstEvent.type){
        case SDL_MOUSEBUTTONUP:
            {
                if(In(rstEvent.button.x, rstEvent.button.y)){
                    if(m_OnClickDone){ m_OnClick(); }
                    m_State = 1;
                    return true;
                }else{
                    m_State = 0;
                    return false;
                }
                break;
            }

        case SDL_MOUSEBUTTONDOWN:
            {
                if(In(rstEvent.button.x, rstEvent.button.y)){
                    if(!m_OnClickDone){ m_OnClick(); }
                    m_State = 2;
                    return true;
                }else{
                    m_State = 0;
                    return false;
                }
                break;
            }
        case SDL_MOUSEMOTION:
            {
                if(In(rstEvent.motion.x, rstEvent.motion.y)){
                    if(m_State != 2){
                        m_State = 1;
                    }
                    return true;
                }else{
                    m_State = 0;
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
