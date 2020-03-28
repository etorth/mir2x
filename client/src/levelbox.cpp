/*
 * =====================================================================================
 *
 *       Filename: levelbox.cpp
 *        Created: 03/28/2020 05:47:00
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

#include "levelbox.hpp"

bool levelBox::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return false;
    }

    if(!In(event.motion.x, event.motion.y)){
        return valid;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(event.button.clicks == 2){
                    m_onDoubleClick();
                }
                break;
            }
        case SDL_MOUSEMOTION:
            {
                if(In(event.motion.x, event.motion.y) && (event.motion.state & SDL_BUTTON_LMASK)){
                    m_onDrag(event.motion.yrel);
                }
                break;
            }
        default:
            {
                break;
            }
    }
    return false;
}
