/*
 * =====================================================================================
 *
 *       Filename: button.cpp
 *        Created: 8/21/2015 4:12:57 AM
 *  Last Modified: 09/03/2015 2:30:59 AM
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
#include "button.hpp"
#include "texturemanager.hpp"
#include "devicemanager.hpp"

Button::Button(int nPreCode, int nFileIndex, int nImageIndex,
        std::function<void()> fnOnClick)
    : Button(
            nPreCode, nFileIndex, nImageIndex + 0,
            nPreCode, nFileIndex, nImageIndex + 1,
            nPreCode, nFileIndex, nImageIndex + 2,
            fnOnClick)
{}

Button::Button(int nPreCode0, int nFileIndex0, int nImageIndex0,
        int nPreCode1, int nFileIndex1, int nImageIndex1,
        int nPreCode2, int nFileIndex2, int nImageIndex2,
        std::function<void()> fnOnClick)
    : Widget()
    , m_OnClick(fnOnClick)
    , m_State(0)
{
	m_Texture[0] = GetTextureManager()->RetrieveTexture(nPreCode0, nFileIndex0, nImageIndex0);
	m_Texture[1] = GetTextureManager()->RetrieveTexture(nPreCode1, nFileIndex1, nImageIndex1);
	m_Texture[2] = GetTextureManager()->RetrieveTexture(nPreCode2, nFileIndex2, nImageIndex2);

	int nW, nH;
    for(int nCnt = 0; nCnt < 3; ++nCnt){
		SDL_QueryTexture(m_Texture[nCnt], nullptr, nullptr, &nW, &nH);
		m_W = (std::max)(m_W, nW);
		m_H = (std::max)(m_H, nH);
    }
}

Button::~Button()
{}

void Button::Draw()
{
    if(m_Texture[m_State]){
        SDL_Rect stDst = { 
            X() + (int)(m_State == 2),
            Y() + (int)(m_State == 2),
            W(), H() };
        SDL_RenderCopy(GetDeviceManager()->GetRenderer(),
                m_Texture[m_State], nullptr, &stDst);
    }
}

void Button::Update(Uint32)
{}

bool Button::HandleEvent(SDL_Event &stEvent)
{
    switch(stEvent.type){
        case SDL_MOUSEBUTTONUP:
            {
                if(In(stEvent.button.x, stEvent.button.y)){
                    if(m_OnClick){
                        m_OnClick();
                    }
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
