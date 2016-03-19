/*
 * =====================================================================================
 *
 *       Filename: inputbox.cpp
 *        Created: 8/21/2015 7:04:16 PM
 *  Last Modified: 03/15/2016 00:02:59
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

#include <SDL2/SDL.h>
#include <utf8.h>
#include <algorithm>
#include "inputbox.hpp"
#include "sdlkeyeventchar.hpp"
#include "mathfunc.hpp"

int IDBox::m_ShowSystemCursorCount = 0;
int IDBox::m_InputBoxCount         = 0;

IDBox::IDBox(int nW, int nH,
        uint8_t nFontSet, uint8_t nFontSize, uint32_t nTextColor,
        int nCursorWidth, uint32_t nCursorColor)
    : Widget()
    , m_CursorWidth(nCursorWidth)
    , m_CursorColor(nCursorColor)
    , m_FontSet(nFontSet)
    , m_Size(nFontSize)
    , m_TextColor(nTextColor)
    , m_SystemCursorX(0)
    , m_SystemCursorY(0)
    , m_DrawOwnSystemCursor(false)
    , m_ShowStartX(0)
    , m_Ticks(0)
    , m_Focus(false)
    , m_Content("")
    , m_IME(nullptr)
{
    m_InputBoxCount++;
    m_ShowSystemCursorCount++;
    m_W = nW;
    m_H = nH;

    SetProperH();
}

IDBox::~IDBox()
{
    m_InputBoxCount--;
    m_ShowSystemCursorCount--;
}

void IDBox::Update(Uint32 nMs)
{
    m_Ticks += nMs;
}

bool IDBox::ProcessEvent(const SDL_Event &rstEvent)
{
    switch(rstEvent.type){
        case SDL_MOUSEMOTION:
            {
                if(In(rstEvent.motion.x, rstEvent.motion.y)){
                    m_SystemCursorX    = rstEvent.motion.x;
                    m_SystemCursorY    = rstEvent.motion.y;
                    if(!m_DrawOwnSystemCursor){
                        m_ShowSystemCursorCount--;
                    }
                    m_DrawOwnSystemCursor = true;
                }else{
                    if(m_DrawOwnSystemCursor){
                        m_ShowSystemCursorCount++;
                    }
                    m_DrawOwnSystemCursor = false;
                }
                break;
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(In(rstEvent.button.x, rstEvent.button.y)){
                    m_Focus = true;
                    BindCursorTokenBox(rstEvent.button.x, rstEvent.button.y);
                    ResetShowStartX();
                    return true;
                }else{
                    m_Focus = false;
                }
                break;
            }
        case SDL_KEYDOWN:
            {
                if(m_Focus){
                    // clear the count
                    m_Ticks = 0;


                    if(m_IME){
                        return m_IME->ProcessEvent(rstEvent);
                    }

                    char chKeyName = SDLKeyEventChar(rstEvent);


                    if(chKeyName != '\0'){
                        m_BindTokenBoxIndex++;
                        m_Content.insert((std::size_t)(m_BindTokenBoxIndex), (std::size_t)1, chKeyName);
                        Compile();
                        ResetShowStartX();
                        return true;
                    }

                    if(rstEvent.key.keysym.sym == SDLK_BACKSPACE){
                        if(m_BindTokenBoxIndex >= 0){
                            m_Content.erase(m_BindTokenBoxIndex, 1);
                            m_BindTokenBoxIndex--;
                            Compile();
                            ResetShowStartX();
                        }
                        return true;
                    }

                    if(rstEvent.key.keysym.sym == SDLK_LEFT){
                        if(m_BindTokenBoxIndex >= 0){
                            m_BindTokenBoxIndex--;
                            ResetShowStartX();
                        }
                        return true;
                    }

                    if(rstEvent.key.keysym.sym == SDLK_RIGHT){
                        if((size_t)(m_BindTokenBoxIndex + 1) < m_Content.size()){
                            m_BindTokenBoxIndex++;
                            ResetShowStartX();
                        }
                        return true;
                    }
                }
                break;
            }
        default:
            break;
    }
    return false;
}


void IDBox::BindCursorTokenBox(int nEventX, int nEventY)
{
    int nX = nEventX - X() + m_ShowStartX;
    int nY = nEventY - Y();

    for(int nIndex = 0; nIndex < (int)m_Line.size(); ++nIndex){
        int nBoxStartX = m_Line[nIndex].Cache.StartX - m_Line[nIndex].State.W1;
        int nBoxStartY = m_Line[nIndex].Cache.StartY;
        int nBoxW      = m_Line[nIndex].State.W1 + m_Line[nIndex].Cache.W + m_Line[nIndex].State.W2;
        int nBoxH      = m_Line[nIndex].Cache.H;
        if(PointInRectangle(nX, nY, nBoxStartX, nBoxStartY, nBoxW, nBoxH)){
            if(PointInRectangle(nX, nY, nBoxStartX, nBoxStartY, nBoxW / 2, nBoxH)){
                m_BindTokenBoxIndex = nIndex - 1;
            }else{
                m_BindTokenBoxIndex = nIndex;
            }
            return;
        }
    }
    m_BindTokenBoxIndex = m_Line.size() - 1;
}

void IDBox::ResetShowStartX()
{
    if(m_BindTokenBoxIndex < 0){
        m_ShowStartX = 0;
        return;
    }

    if(m_Line[m_BindTokenBoxIndex].Cache.StartX < m_ShowStartX){
        m_ShowStartX = m_Line[m_BindTokenBoxIndex].Cache.StartX
            - m_Line[m_BindTokenBoxIndex].State.W1;
        return;
    }

    int nBoxEndX = m_Line[m_BindTokenBoxIndex].Cache.StartX
        + m_Line[m_BindTokenBoxIndex].State.W1
        + m_Line[m_BindTokenBoxIndex].State.W2
        + m_Line[m_BindTokenBoxIndex].Cache.W - 1;

    if(nBoxEndX > m_ShowStartX + m_W - 1){
        m_ShowStartX = nBoxEndX - m_W + 1;
        return;
    }

    // otherwise m_BindTokenBoxIndex is ok
    // do nothing ??
}


void IDBox::Draw()
{
    if(m_TokenBoard.W() + m_CursorWidth <= m_W){
        m_TokenBoard.Draw(X(), Y());
    }else{
        m_TokenBoard.Draw(X(), Y(), 
                m_W - m_TokenBoard.W() - m_CursorWidth, 0);
    }
}

void IDBox::Draw()
{
    int  nX = X();
    int  nY = Y();
    bool bStartDraw = false;

    for(int nIndex = 0; nIndex < m_Line.size(); ++nIndex){
        int nBoxStartX = m_Line[nIndex].Cache.StartX;
        int nBoxStopX  = m_Line[nIndex].Cache.StartX + m_Line[nIndex].Cache.W - 1;

        if(nBoxStopX >= m_ShowStartX || nBoxStartX <= (m_ShowStartX + m_W - 1)){
            int nCutX1         = (std::max)(0, m_ShowStartX - nBoxStartX);
            int nCutX2         = (std::max)(0, nBoxStopX - (m_ShowStartX + m_W - 1));
            int nDrawBoxStartX = nCutX1;
            int nDrawBoxStopX  = (m_Line[nIndex].Cache.W - 1) - nCutX2;

            SDL_Rect stSrc, stDst;

            stSrc.x = nDrawBoxStartX;
            stSrc.y = 0;
            stSrc.w = nDrawBoxStopX - nDrawBoxStartX + 1;
            stSrc.h = m_Line[nIndex].Cache.H;

            stDst.x = nX + m_Line[nIndex].Cache.StartX + stSrc.x - m_ShowStartX;
            stDst.y = nY + (m_H - m_Line[nIndex].Cache.H);
            stDst.w = stSrc.w;
            stDst.h = stSrc.h;

            SDL_RenderCopy(GetDeviceManager()->GetRenderer(),
                    m_Line[nIndex].UTF8CharBox.Cache.Texture[0],
                    &stSrc, &stDst);
            bStartDraw = true;
        }else{
            if(bStartDraw){
                return;
            }
        }
    }
    DrawCursor();
    DrawSystemCursor();
}

void IDBox::DrawCursor()
{
    if(m_Ticks % 1000 < 500 && m_Focus){
        int nX, nY, nH;
        if(m_BindTokenBoxIndex == -1){
            nX = X();
        }else{
            nX = X()
                + m_Line[m_BindTokenBoxIndex].Cache.StartX
                + m_Line[m_BindTokenBoxIndex].Cache.W
                + m_Line[m_BindTokenBoxIndex].State.W1
                - m_ShowStartX;
        }
        nY = Y() - 1;
        nH = m_H + 2;
        GetDeviceManager()->SetRenderDrawColor(200, 200, 200, 200);
        SDL_RenderDrawLine(
                GetDeviceManager()->GetRenderer(),
                nX, nY, nX, nY + nH);
        // SDL_RenderDrawLine(
        //         GetDeviceManager()->GetRenderer(),
        //         nX - (std::max)(1, (int)std::lround(nH * 0.05)), nY,
        //         nX + (std::max)(1, (int)std::lround(nH * 0.05)), nY);
        // SDL_RenderDrawLine(
        //         GetDeviceManager()->GetRenderer(),
        //         nX - (std::max)(1, (int)std::lround(nH * 0.05)), nY + nH,
        //         nX + (std::max)(1, (int)std::lround(nH * 0.05)), nY + nH);
        GetDeviceManager()->SetRenderDrawColor(0, 0, 0, 0);
    }
}

void IDBox::DrawSystemCursor()
{
    if(m_DrawOwnSystemCursor){
        GetDeviceManager()->SetRenderDrawColor(200, 200, 200, 200);
        SDL_RenderDrawLine(
                GetDeviceManager()->GetRenderer(),
                m_SystemCursorX, m_SystemCursorY - m_H / 2,
                m_SystemCursorX, m_SystemCursorY + m_H / 2);
        SDL_RenderDrawLine(
                GetDeviceManager()->GetRenderer(),
                m_SystemCursorX - (std::max)(1, (int)std::lround(m_H * 0.08)),
                m_SystemCursorY - m_H / 2,
                m_SystemCursorX + (std::max)(1, (int)std::lround(m_H * 0.08)),
                m_SystemCursorY - m_H / 2);
        SDL_RenderDrawLine(
                GetDeviceManager()->GetRenderer(),
                m_SystemCursorX - (std::max)(1, (int)std::lround(m_H * 0.08)),
                m_SystemCursorY + m_H / 2,
                m_SystemCursorX + (std::max)(1, (int)std::lround(m_H * 0.08)),
                m_SystemCursorY + m_H / 2);
        GetDeviceManager()->SetRenderDrawColor(0, 0, 0, 0);
    }

    if(m_ShowSystemCursorCount < m_InputBoxCount){
        SDL_ShowCursor(0);
    }else{
        SDL_ShowCursor(1);
    }
}

void IDBox::Compile()
{
    m_XMLContent.clear();

    m_Line.clear();

    TOKENBOX stTokenBox;

    const char *pStart = m_Content.c_str();
    const char *pEnd   = pStart;

    while(*pEnd != '\0'){
        pStart = pEnd;
        utf8::unchecked::advance(pEnd, 1);
        std::memset(stTokenBox.UTF8CharBox.Data, 0, 8);
        if(pEnd - pStart == 1 && (*pStart == '\n' || *pStart == '\t' || *pStart == '\r')){
            // continue;
            stTokenBox.UTF8CharBox.Data[0] = ' ';
        }else{
            std::memcpy(stTokenBox.UTF8CharBox.Data, pStart, pEnd - pStart);
        }
        LoadUTF8CharBoxCache(stTokenBox);
        PushBack(stTokenBox);
    }
    SetTokenBoxStartX();
}

void IDBox::SetTokenBoxStartX()
{
    int nCurrentX = 0;
    for(auto &stTokenBox: m_Line){
        nCurrentX += stTokenBox.State.W1;
        stTokenBox.Cache.StartX = nCurrentX;
        nCurrentX += stTokenBox.Cache.W;
        nCurrentX += stTokenBox.State.W2;
    }
}

void IDBox::PushBack(TOKENBOX &stTokenBox)
{
    stTokenBox.State.W1 = 0;
    // stTokenBox.State.W2 = 0;
    stTokenBox.State.W2 = 1;
    m_Line.push_back(stTokenBox);
}

void IDBox::LoadUTF8CharBoxCache(TOKENBOX &stTokenBox)
{
    std::memcpy(m_CharBoxCache.Data, stTokenBox.UTF8CharBox.Data, 8);

    stTokenBox.UTF8CharBox.Cache.Texture[0] = GetFontTextureManager()->RetrieveTexture(m_CharBoxCache);
    SDL_QueryTexture(stTokenBox.UTF8CharBox.Cache.Texture[0],
            nullptr, nullptr, &(stTokenBox.Cache.W), &(stTokenBox.Cache.H));
    stTokenBox.Cache.H1     = stTokenBox.Cache.H;
    stTokenBox.Cache.H2     = 0;
    stTokenBox.Cache.StartY = 0;

    m_H = (std::max)(m_H, stTokenBox.Cache.H);
}

void IDBox::SetContent(const char *szInfo)
{
    if(szInfo){
        m_Content = szInfo;
    }else{
        m_Content = "";
    }
    Compile();
    m_BindTokenBoxIndex = m_Content.size() - 1;
    ResetShowStartX();
}

const char *IDBox::Content()
{
    return m_Content.c_str();
}

void IDBox::SetProperH()
{
    TOKENBOX stTokenBox;
    stTokenBox.UTF8CharBox.Data[0] = ' ';
    stTokenBox.UTF8CharBox.Data[1] = '\0';
    LoadUTF8CharBoxCache(stTokenBox);
}
