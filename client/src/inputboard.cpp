/*
 * =====================================================================================
 *
 *       Filename: inputboard.cpp
 *        Created: 08/21/2015 07:04:16 PM
 *  Last Modified: 08/14/2016 13:39:02
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
#include "game.hpp"
#include "inputboard.hpp"
#include "sdlkeyeventchar.hpp"
#include "fontexdbn.hpp"
#include "mathfunc.hpp"
#include "supwarning.hpp"

int InputBoard::s_ShowSystemCursorCount = 0;
int InputBoard::s_InputBoardCount       = 0;

void InputBoard::Update(double fMS)
{
    m_MS += fMS;

    InputWidget::Update(fMS);
    m_TokenBoard.Update(fMS);
}

bool InputBoard::ProcessEvent(const SDL_Event &rstEvent, bool *bValid)
{
    if(bValid && !(*bValid)){ return false; }

    switch(rstEvent.type){
        case SDL_MOUSEMOTION:
            {
                if(In(rstEvent.motion.x, rstEvent.motion.y)){
                    m_SystemCursorX    = rstEvent.motion.x;
                    m_SystemCursorY    = rstEvent.motion.y;
                    if(!m_DrawOwnSystemCursor){
                        s_ShowSystemCursorCount--;
                    }
                    m_DrawOwnSystemCursor = true;
                }else{
                    if(m_DrawOwnSystemCursor){
                        s_ShowSystemCursorCount++;
                    }
                    m_DrawOwnSystemCursor = false;
                }

                bool bInnValid = true;
                m_TokenBoard.ProcessEvent(rstEvent, &bInnValid);
                break;
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(In(rstEvent.button.x, rstEvent.button.y)){
                    bool bInnValid = true;
                    m_TokenBoard.ProcessEvent(rstEvent, &bInnValid);

                    ResetTokenBoardLocation();
                    m_Focus = true;
                    return true;
                }else{
                    m_Focus = false;
                }
                break;
            }
        case SDL_KEYDOWN:
            {
                if(m_IME && m_IME->Focus()){
                    // if IME is working
                    // then all input events should be grabbed by IME
                    return false;
                }

                // if IME is disabled, then input board handles key down
                // tokenboard won't handle this event
                if(Focus()){
                    // clear the count no matter what key pressed
                    m_MS = 0.0;
                    switch(rstEvent.key.keysym.sym){
                        case SDLK_UP:
                            {
                                int nX, nY;
                                m_TokenBoard.GetCursor(&nX, &nY);
                                m_TokenBoard.SetCursor(nX, nY - 1);
                                break;
                            }
                        case SDLK_DOWN:
                            {
                                int nX, nY;
                                m_TokenBoard.GetCursor(&nX, &nY);
                                m_TokenBoard.SetCursor(nX, nY + 1);
                                break;
                            }

                        case SDLK_LEFT:
                            {
                                int nX, nY;
                                m_TokenBoard.GetCursor(&nX, &nY);
                                m_TokenBoard.SetCursor(nX - 1, nY);
                                break;
                            }

                        case SDLK_RIGHT:
                            {
                                int nX, nY;
                                m_TokenBoard.GetCursor(&nX, &nY);
                                m_TokenBoard.SetCursor(nX + 1, nY);
                                break;
                            }

                        case SDLK_BACKSPACE:

                            {
                                m_TokenBoard.Delete(false);
                                break;
                            }

                        case SDLK_ESCAPE:
                            {
                                Focus(false);
                                break;
                            }

                        case SDLK_x:
                            {
                                if(false
                                        || rstEvent.key.keysym.mod & KMOD_LCTRL
                                        || rstEvent.key.keysym.mod & KMOD_RCTRL){
                                    extern Game *g_Game;
                                    g_Game->Clipboard(m_TokenBoard.GetXML(true));
                                    m_TokenBoard.Delete(true);
                                }else{
                                    m_TokenBoard.AddUTF8Code(uint32_t('x'));
                                }
                                break;
                            }

                        case SDLK_c:
                            {
                                if(false
                                        || rstEvent.key.keysym.mod & KMOD_LCTRL
                                        || rstEvent.key.keysym.mod & KMOD_RCTRL){
                                    extern Game *g_Game;
                                    g_Game->Clipboard(m_TokenBoard.GetXML(true));
                                }else{
                                    m_TokenBoard.AddUTF8Code(uint32_t('c'));
                                }
                                break;
                            }

                        case SDLK_v:
                            {
                                if(false
                                        || rstEvent.key.keysym.mod & KMOD_LCTRL
                                        || rstEvent.key.keysym.mod & KMOD_RCTRL){
                                    extern Game *g_Game;
                                    m_TokenBoard.ParseXML(g_Game->Clipboard().c_str());
                                }else{
                                    m_TokenBoard.AddUTF8Code(uint32_t('v'));
                                }
                                break;
                            }

                        default:
                            {
                                // end of special event handle, normal input
                                char chKeyName = SDLKeyEventChar(rstEvent);
                                if(chKeyName != '\0'){
                                    m_TokenBoard.AddUTF8Code(uint32_t(chKeyName));
                                }
                                break;
                            }

                    }

                    ResetTokenBoardLocation();
                    return true;
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

void InputBoard::GetCursorInfo(int *pX, int *pY, int *pW, int *pH)
{
    // +------------------------------+
    // |                              |
    // |                       +-+    |
    // |                       | |    |
    // |                       | |    |
    // |                       | |    |
    // |                       | |    |
    // |                       | |    |
    // |                       +-+    |
    // |                              |
    // +------------------------------+
    //                          ^
    //                          |
    //
    // cursor with non-zero width, but tokenboard itself have no
    // concept of ``width of cursor", to tokenboard cursor is a
    // location for (x, y)
    //
    int nTBX, nTBY, nTBW, nTBH, nX, nY, nW, nH;

    nW = m_CursorWidth;
    m_TokenBoard.GetCursor(&nX, &nY);

    if(m_TokenBoard.GetTokenBoxInfo(nX - 1, nY,
                nullptr, &nTBX, &nTBY, &nTBW, &nTBH, nullptr, nullptr)){
        nX = nTBX + nTBW;
        nY = nTBY;
        nH = nTBH + 2 * m_TokenBoard.GetLineSpace();
    }else if(m_TokenBoard.GetTokenBoxInfo(nX, nY,
                nullptr, &nTBX, &nTBY, &nTBW, &nTBH, nullptr, nullptr)){
        nX = nTBX + nTBW;
        nY = nTBY;
        nH = nTBH + 2 * m_TokenBoard.GetLineSpace();
    }else{
        // should be the empty tokenboard, waiting for the first input
        // use the default setting
        //
        // only need to get default font/size/style(style for italy)
        uint8_t nFont, nFontSize, nFontStyle;
        m_TokenBoard.GetDefaultFontInfo(&nFont, &nFontSize, &nFontStyle);

        extern FontexDBN *g_FontexDBN;
        auto pTexture = g_FontexDBN->Retrieve(nFont, nFontSize, nFontStyle, (int)'M'); 

        int nDefaultH;
        SDL_QueryTexture(pTexture, nullptr, nullptr, nullptr, &nDefaultH);
        nX = 0;
        nY = 0;
        nH = nDefaultH;
    }

    if(*pX){ *pX = nX; }
    if(*pY){ *pY = nY; }
    if(*pW){ *pW = nW; }
    if(*pH){ *pH = nH; }
}

// we always move the cursor point into the visiable region
// in this model inputboard is a view window of the tokenboard
//
void InputBoard::ResetTokenBoardLocation()
{
    int nX, nY, nW, nH;
    GetCursorInfo(&nX, &nY, &nW, &nH);

    int nTokenBoardX = m_TokenBoard.X();
    int nTokenBoardY = m_TokenBoard.Y();

    int nInputBoardX = X();
    int nInputBoardY = Y();

    // get the relative view window on the tokenboard
    int nDX = nInputBoardX - nTokenBoardX;
    int nDY = nInputBoardY - nTokenBoardY;
    int nDW = W();
    int nDH = H();

    if(!RectangleInside(nDX, nDY, nDW, nDH, nX, nY, nW, nH)){
        // cursor is not contained in the view window, need to be reset
        if(nX < nDX){
            m_TokenBoard.Move(nDX - nX, 0);
        }

        if(nX + nW > nDX + nDW){
            m_TokenBoard.Move(nX + nW - nDX - nDW, 0);
        }

        if(nY < nDY){
            m_TokenBoard.Move(0, nDY - nY);
        }

        if(nY + nW > nDY + nDH){
            m_TokenBoard.Move(0, nY + nW - nDY - nDH);
        }
    }
}

void InputBoard::Draw()
{
    if(RectangleInside(X(), Y(), W(), H(),
                m_TokenBoard.X(), m_TokenBoard.Y(),
                m_TokenBoard.W(), m_TokenBoard.H())){
        m_TokenBoard.Draw(X(), Y());
    }else{
        int nTokenBoardX = m_TokenBoard.X();
        int nTokenBoardY = m_TokenBoard.Y();

        int nInputBoardX = X();
        int nInputBoardY = Y();

        // get the relative view window on the tokenboard
        int nDX = nInputBoardX - nTokenBoardX;
        int nDY = nInputBoardY - nTokenBoardY;
        int nDW = W();
        int nDH = H();


        // clip to draw
        // m_TokenBoard.DrawEx(X(), Y(), nDX, nDY, nDW, nDH);
        UNUSED(nDX);
        UNUSED(nDY);
        m_TokenBoard.DrawEx(X(), Y(), 0, 0, nDW, nDH);
    }

    // +-------------------------+
    // |                         |
    // |      XXXXXX | XXXXXX    |
    // |      I                  |
    // +-------------------------+
    //
    // | : blinking cursor in the box
    // I : mouse pointer to select input cursor position

    // 1. draw ``|" cursor
    int nX, nY, nW, nH;
    GetCursorInfo(&nX, &nY, &nW, &nH);

    if(((int)m_MS % 1000) < 500 && Focus()){
        extern SDLDevice   *g_SDLDevice;
        g_SDLDevice->PushColor(m_CursorColor.r, m_CursorColor.g, m_CursorColor.b, m_CursorColor.a);
        g_SDLDevice->FillRectangle(nX, nY, nW, nH);
    }

    // 2. draw ``I" cursor
    if(m_DrawOwnSystemCursor){
        extern SDLDevice   *g_SDLDevice;
        g_SDLDevice->PushColor(200, 200, 200, 200);
        g_SDLDevice->DrawLine(
                m_SystemCursorX, m_SystemCursorY - m_H / 2,
                m_SystemCursorX, m_SystemCursorY + m_H / 2);
        g_SDLDevice->DrawLine(
                m_SystemCursorX - (std::max)(1, (int)std::lround(m_H * 0.08)),
                m_SystemCursorY - m_H / 2,
                m_SystemCursorX + (std::max)(1, (int)std::lround(m_H * 0.08)),
                m_SystemCursorY - m_H / 2);
        g_SDLDevice->DrawLine(
                m_SystemCursorX - (std::max)(1, (int)std::lround(m_H * 0.08)),
                m_SystemCursorY + m_H / 2,
                m_SystemCursorX + (std::max)(1, (int)std::lround(m_H * 0.08)),
                m_SystemCursorY + m_H / 2);
        g_SDLDevice->PopColor();
    }

    if(s_ShowSystemCursorCount < s_InputBoardCount){
        SDL_ShowCursor(0);
    }else{
        SDL_ShowCursor(1);
    }
}
