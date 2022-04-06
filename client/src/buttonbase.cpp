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
#include "sdldevice.hpp"
#include "soundeffectdb.hpp"

extern SDLDevice *g_sdlDevice;
extern SoundEffectDB *g_seffDB;

bool ButtonBase::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        if(getState() != BEVENT_OFF){
            setState(BEVENT_OFF);
            onOverOut();
        }
        return focusConsume(this, false);
    }

    if(!m_active){
        if(getState() != BEVENT_OFF){
            setState(BEVENT_OFF);
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

    if(m_seffID[2] != SYS_U32NIL){
        g_sdlDevice->playSoundEffect(g_seffDB->retrieve((m_seffID[2])));
    }
}

void ButtonBase::onOverIn()
{
    if(m_onOverIn){
        m_onOverIn();
    }

    if(m_seffID[0] != SYS_U32NIL){
        g_sdlDevice->playSoundEffect(g_seffDB->retrieve((m_seffID[0])));
    }
}

void ButtonBase::onOverOut()
{
    if(m_onOverOut){
        m_onOverOut();
    }

    if(m_seffID[1] != SYS_U32NIL){
        g_sdlDevice->playSoundEffect(g_seffDB->retrieve((m_seffID[1])));
    }
}

void ButtonBase::onBadEvent()
{
}
