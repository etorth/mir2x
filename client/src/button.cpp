/*
 * =====================================================================================
 *
 *       Filename: button.cpp
 *        Created: 08/21/2015 04:12:57
 *  Last Modified: 03/07/2016 23:45:11
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

Button::Button(int nFileIndex, int nImageIndex,
        std::function<void()> fnOnClick,
        std::function<bool(uint32_t, int &, int &)> fnTextureInfo)
    : Widget()
    , m_TextureID(((nFileIndex & 0XFF) << 16) + (nImageIndex & 0XFF))
    , m_State(0)
    , m_OnClick(fnOnClick)
{
    // the reason here I use fnTextureInfo rather than SDL_QueryTexture
    // is because that
    // Button class is in Process* class
    // which can't access SDL_Renderer directly
    //

    // for(int nCnt = 0; nCnt < 3; ++nCnt){
    //     int nW, nH;
    //     if(fnTextureInfo(m_TextureID + (uint32_t)nCnt), nW, nH){
    //         m_W = (std::max)(m_W, nW);
    //         m_H = (std::max)(m_H, nH);
    //     }else{
    //         throw std::invalid_argument("invalid texture id");
    //     }
    // }

    if(!fnTextureInfo(m_TextureID, m_W, m_H)){
        throw std::invalid_argument("invalid texture ID");
    }
}

Button::~Button()
{}

void Button::Draw(std::function<void(uint32_t, int, int)> fnDraw)
{
    fnDraw(m_TextureID + (uint32_t)m_State,
            X() + (int)(m_State == 2), Y() + (int)(m_State == 2));
}

void Button::Update(uint32_t nMS)
{
    m_MS += (int)nMS;
}

bool Button::ProcessEvent(SDL_Event &stEvent)
{
    switch(stEvent.type){
        case SDL_MOUSEBUTTONUP:
            {
                if(In(stEvent.button.x, stEvent.button.y)){
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
                if(In(stEvent.button.x, stEvent.button.y)){
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
                if(In(stEvent.motion.x, stEvent.motion.y)){
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
