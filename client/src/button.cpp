/*
 * =====================================================================================
 *
 *       Filename: button.cpp
 *        Created: 08/21/2015 04:12:57
 *  Last Modified: 03/20/2016 21:28:51
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
#include "button.hpp"
#include "pngtexdbn.hpp"
#include "sdldevice.hpp"
#include "log.hpp"

Button::Button(uint8_t nFileIndex,  uint16_t nImageIndex,
        int nX, int nY, const std::function<void()> &fnOnClick)
    : Widget()
    , m_BaseID((((uint32_t)nFileIndex) << 16) + nImageIndex)
    , m_State(0)
    , m_OnClick(fnOnClick)
{
    extern Log       *g_Log;
    extern PNGTexDBN *g_PNGTexDBN;
    extern SDLDevice *g_SDLDevice;

    m_X = nX;
    m_Y = nY;

    auto pTexture = g_PNGTexDBN->Retrieve(m_BaseID);
    if(pTexture){
        if(SDL_QueryTexture(pTexture, nullptr, nullptr, &m_W, &m_H)){
            g_Log->AddLog(LOGTYPE_INFO, "Button(%d, %d): X = %d, Y = %d, W = %d, H = %d",
                    nFileIndex, nImageIndex, m_X, m_Y, m_W, m_H);
        }
    }

    g_SDLDevice->DrawTexture(
            g_PNGTexDBN->Retrieve(m_BaseID + (uint32_t)m_State),
            X() + (int)(m_State == 2),
            Y() + (int)(m_State == 2));
}

void Button::Draw()
{
    extern PNGTexDBN *g_PNGTexDBN;
    extern SDLDevice *g_SDLDevice;

    g_SDLDevice->DrawTexture(
            g_PNGTexDBN->Retrieve(m_BaseID + (uint32_t)m_State),
            X() + (int)(m_State == 2),
            Y() + (int)(m_State == 2));
}

bool Button::ProcessEvent(const SDL_Event &rstEvent)
{
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
