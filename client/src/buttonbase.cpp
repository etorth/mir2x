/*
 * =====================================================================================
 *
 *       Filename: buttonbase.cpp
 *        Created: 08/21/2015 04:12:57
 *  Last Modified: 03/16/2017 14:09:24
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

#include <stdexcept>
#include <algorithm>
#include <functional>

#include "log.hpp"
#include "button.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"


void Button::Draw(int nX, int nY)
{
    extern PNGTexDBN *g_PNGTexDBN;
    extern SDLDevice *g_SDLDevice;

    g_SDLDevice->DrawTexture(
            g_PNGTexDBN->Retrieve(m_BaseID + (uint32_t)m_State),
            nX + (int)(m_State == 2),
            nY + (int)(m_State == 2));
}

bool Button::ProcessEvent(const SDL_Event &rstEvent, bool *bValid)
{
    if(bValid && !(*bValid)){ return false; }

    switch(rstEvent.type){
        case SDL_MOUSEBUTTONUP:
            {
                if(In(rstEvent.button.x, rstEvent.button.y)){
                    m_OnClick();
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
            return false;
    }
    return false;
}
