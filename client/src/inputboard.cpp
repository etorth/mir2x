/*
 * =====================================================================================
 *
 *       Filename: inputboard.cpp
 *        Created: 08/21/2015 7:04:16 PM
 *  Last Modified: 03/26/2016 17:41:57
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
#include "inputboard.hpp"
#include "sdlkeyeventchar.hpp"

int InputBoard::s_ShowSystemCursorCount = 0;
int InputBoard::s_InputBoardCount       = 0;

InputBoard::InputBoard(bool bWrap, int nW, int nH,
        int nMinTextMargin, int nMinTextLineSpace,
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
    , m_TokenBoard(bWrap, nW, nMinTextMargin, nMinTextLineSpace)
    , m_IME(nullptr)
{
    s_InputBoardCount++;
    s_ShowSystemCursorCount++;
    m_W = nW;
    m_H = nH;

    SetProperH();
}

InputBoard::~InputBoard()
{
    s_InputBoardCount--;
    s_ShowSystemCursorCount--;
}

void InputBoard::Update(Uint32 nMs)
{
    m_Ticks += nMs;
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

                    ResetShowStartX();
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
                                m_TokenBoard->GetCursor(&nX, &nY);
                                m_TokenBoard->SetCursor(nX, nY - 1);
                                break;
                            }
                        case SDLK_DOWN:
                            {
                                int nX, nY;
                                m_TokenBoard->GetCursor(&nX, &nY);
                                m_TokenBoard->SetCursor(nX, nY + 1);
                                break;
                            }

                        case SDLK_LEFT:
                            {
                                int nX, nY;
                                m_TokenBoard->GetCursor(&nX, &nY);
                                m_TokenBoard->SetCursor(nX - 1, nY);
                                break;
                            }

                        case SDLK_RIGHT:
                            {
                                int nX, nY;
                                m_TokenBoard->GetCursor(&nX, &nY);
                                m_TokenBoard->SetCursor(nX + 1, nY);
                                break;
                            }

                        case SDLK_BACKSPACE:

                            {
                                m_TokenBoard->Delete(false);
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
                                    g_Game->Clipboard(m_TokenBoard->GetXML(true));
                                    m_TokenBoard->Delete(true);
                                }
                                break;
                            }

                        case SDL_c:
                            {
                                if(false
                                        || rstEvent.key.keysym.mod & KMOD_LCTRL
                                        || rstEvent.key.keysym.mod & KMOD_RCTRL){
                                    extern Game *g_Game;
                                    g_Game->Clipboard(m_TokenBoard->GetXML(true));
                                }
                                break;
                            }

                        case SDL_v:
                            {
                                if(false
                                        || rstEvent.key.keysym.mod & KMOD_LCTRL
                                        || rstEvent.key.keysym.mod & KMOD_RCTRL){
                                    extern Game *g_Game;
                                    m_TokenBoard->InsertXML(g_Game->Clipboard());
                                }
                                break;
                            }

                        default:
                            {
                                // end of special event handle, normal input
                                chKeyName = SDLKeyEventChar(rstEvent);
                                if(chKeyName != '\0'){
                                    m_TokenBoard->AddUTF8Char(int(chKeyName));
                                    return true;
                                }
                            }

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


void InputBoard::BindCursorTokenBox(int nEventX, int nEventY)
{
    // +-------------------------- <------- screen corner
    // |
    // |
    // |   +----------------------+ <------ full tokenbox
    // |   |                      |
    // |   |  +--------------+    | <------ the inputboard window
    // |   |  |              |    |
    // |   |  |              |    |
    // |   |  |  x           |    | <------ event point
    // |   |  |              |    |
    // |   |  +--------------+    |
    // |   |                      |
    // |   |                      |
    // |   +----------------------+

    int nRow, nCol;

    if(m_TokenBoard.TokenBoxUnderPoint(nEventX, nEventY, nRow, nCol)){
        m_TokenBoard.TokenBoxLocationInfo(nRow, nCol,
                nStartX, nStartY, nW, nW1, nW2, nH, nH1, nH2);

        m_BindTokenBoxLocation = {nRow, nCol};


    }

    int nX = nEventX - X() + m_ShowStartX;
    int nY = nEventY - Y() + m_ShowStartY;

    for(int nIndex = 0; nIndex < (int)m_Line.size(); ++nIndex){
        int nBoxStartX = m_Line[nIndex].Cache.StartX - m_Line[nIndex].State.W1;
        int nBoxStartY = m_Line[nIndex].Cache.StartY;
        int nBoxW      = m_Line[nIndex].State.W1 + m_Line[nIndex].Cache.W + m_Line[nIndex].State.W2;
        int nBoxH      = m_Line[nIndex].Cache.H;
        if(PointInRect(nX, nY, nBoxStartX, nBoxStartY, nBoxW, nBoxH)){
            if(PointInRect(nX, nY, nBoxStartX, nBoxStartY, nBoxW / 2, nBoxH)){
                m_BindTokenBoxIndex = nIndex - 1;
            }else{
                m_BindTokenBoxIndex = nIndex;
            }
            return;
        }
    }
    m_BindTokenBoxIndex = m_Line.size() - 1;
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
    m_TokenBoard->GetCursor(&nX, &nY);

    if(m_TokenBoard->GetTokenBoxInfo(nX - 1, nY,
                nullptr, &nTBX, &nTBY, &nTBW, &nTBH, nullptr, nullptr)){
        nX = nTBX + nTBW;
        nY = nTBY;
        nH = nTBH + 2 * m_TokenBoard->GetLineSpace();
    }else if(m_TokenBoard->GetTokenBoxInfo(nX, nY,
                nullptr, &nTBX, &nTBY, &nTBW, &nTBH, nullptr, nullptr)){
        nX = nTBX + nTBW;
        nY = nTBY;
        nH = nTBH + 2 * m_TokenBoard->GetLineSpace();
    }else{
        // should be the empty tokenboard, waiting for the first input
        // use the default setting
        extern FontexDBN *g_FontexDBN;
        auto pTexture = g_FontexDBN->Retrieve(
                m_DefaultFontIndex, m_DefaultFontSize, m_DefaultFontStyle, (int)'M');

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
void InputBoard::ResetTokenBoardLoction()
{
    int nX, nY, nW, nH;
    GetCursorInfo(&nX, &nY, &nW, &nH);

    int nTokenBoardX = m_TokenBoard->X();
    int nTokenBoardY = m_TokenBoard->Y();

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
            m_TokenBoard->Move(nDX - nX, 0);
        }

        if(nX + nW > nDX + nDW){
            m_TokenBoard->Move(nX + nW - nDX - nDW, 0);
        }

        if(nY < nDY){
            m_TokenBoard->Move(0, nDY - nY);
        }

        if(nY + nW > nDY + nDH){
            m_TokenBoard->Move(0, nY + nW - nDY - nDH):
        }
    }
}

void InputBoard::Draw()
{
    if(In(m_TokenBoard->X(), m_TokenBoard->Y(),
                m_TokenBoard->W(), m_TokenBoard->H())){
        m_TokenBoard->Draw();
    }else{
        int nTokenBoardX = m_TokenBoard->X();
        int nTokenBoardY = m_TokenBoard->Y();

        int nInputBoardX = X();
        int nInputBoardY = Y();

        // get the relative view window on the tokenboard
        int nDX = nInputBoardX - nTokenBoardX;
        int nDY = nInputBoardY - nTokenBoardY;
        int nDW = W();
        int nDH = H();

        // clip to draw
        m_TokenBoard->Draw(nDX, nDY, nDW, nDH);
    }

    // +-------------------------+
    // |                         |
    // |      XXXXXX | XXXXXX    |
    // |      I                  |
    // +-------------------------+
    //
    // | : blinking cursor in the box
    // I : mouse pointer to select input cursor position

    // 1. draw ``I" cursor
    int nX, nY, nW, nH;
    GetCursorInfo(&nX, &nY, &nW, &nH);

    if(((int)m_MS % 1000) < 500 && Focus()){
        SDL_Rect stRect {nX, nY, nW, nH};
        extern SDLDevice   *g_SDLDevice;
        g_SDLDevice->PushColor(m_CurorColor.r, m_CurorColor.g, m_CurorColor.b, m_CurorColor.a);
        g_SDLDevice->FillRectangle(stRect);
    }

    // 2. draw ``|" cursor

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

    if(s_ShowSystemCursorCount < s_InputBoardCount){
        SDL_ShowCursor(0);
    }else{
        SDL_ShowCursor(1);
    }
}
