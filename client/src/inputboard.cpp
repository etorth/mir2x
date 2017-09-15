/*
 * =====================================================================================
 *
 *       Filename: inputboard.cpp
 *        Created: 08/21/2015 07:04:16
 *  Last Modified: 09/14/2017 20:00:57
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

#include <utf8.h>
#include <algorithm>
#include <SDL2/SDL.h>

#include "game.hpp"
#include "mathfunc.hpp"
#include "fontexdbn.hpp"
#include "inputboard.hpp"
#include "sdlkeyeventchar.hpp"

int InputBoard::s_ShowSystemCursorCount = 0;
int InputBoard::s_InputBoardCount       = 0;

void InputBoard::Update(double fMS)
{
    m_MS += fMS;
    InputWidget::Update(fMS);
    m_TokenBoard.Update(fMS);
}

bool InputBoard::ProcessEvent(const SDL_Event &rstEvent, bool *)
{
    // here we parse all event
    // even some else widget have captured this event

    // for example, A and B are two inputboards
    // if we pressed in A, then B should lost its focus
    //
    // +-------+ +-------+
    // |   A   | |   B   |
    // +-------+ +-------+

    switch(rstEvent.type){
        case SDL_MOUSEMOTION:
            {
                if(In(rstEvent.motion.x, rstEvent.motion.y)){
                    m_SystemCursorX = rstEvent.motion.x;
                    m_SystemCursorY = rstEvent.motion.y;
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
                    Focus(true);
                    return true;
                }else{
                    Focus(false);
                }
                break;
            }
        case SDL_KEYDOWN:
            {
                if(Focus()){
                    // clear the count no matter what key pressed
                    m_MS = 0.0;
                    switch(rstEvent.key.keysym.sym){
                        case SDLK_UP:
                            {
                                int nX, nY;
                                m_TokenBoard.GetCursor(&nX, &nY);
                                if(!m_TokenBoard.SetCursor(nX, nY - 1)){
                                    m_TokenBoard.SetCursor(m_TokenBoard.GetLineTokenBoxCount(nY - 1), nY - 1);
                                }
                                break;
                            }
                        case SDLK_DOWN:
                            {
                                int nX, nY;
                                m_TokenBoard.GetCursor(&nX, &nY);
                                if(!m_TokenBoard.SetCursor(nX, nY + 1)){
                                    m_TokenBoard.SetCursor(m_TokenBoard.GetLineTokenBoxCount(nY + 1), nY + 1);
                                }
                                break;
                            }

                        case SDLK_LEFT:
                            {
                                int nX, nY;
                                m_TokenBoard.GetCursor(&nX, &nY);
                                if(!m_TokenBoard.SetCursor(nX - 1, nY)){
                                    if(nY > 0){
                                        nX = m_TokenBoard.GetLineTokenBoxCount(nY - 1);
                                        m_TokenBoard.SetCursor(nX, nY - 1);
                                    }
                                }
                                break;
                            }

                        case SDLK_RIGHT:
                            {
                                int nX, nY;
                                m_TokenBoard.GetCursor(&nX, &nY);
                                if(!m_TokenBoard.SetCursor(nX + 1, nY)){
                                    if(nY + 1 < m_TokenBoard.GetLineCount()){
                                        m_TokenBoard.SetCursor(0, nY + 1);
                                    }
                                }
                                break;
                            }

                        case SDLK_BACKSPACE:

                            {
                                m_TokenBoard.Delete(false);
                                break;
                            }

                        case SDLK_RETURN:
                            {
                                m_TokenBoard.BreakLine();
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
                                    if(false
                                            || rstEvent.key.keysym.mod & KMOD_LSHIFT
                                            || rstEvent.key.keysym.mod & KMOD_RSHIFT){
                                        m_TokenBoard.AddUTF8Code(uint32_t('X'));
                                    }else{
                                        m_TokenBoard.AddUTF8Code(uint32_t('x'));
                                    }
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
                                    if(false
                                            || rstEvent.key.keysym.mod & KMOD_LSHIFT
                                            || rstEvent.key.keysym.mod & KMOD_RSHIFT){
                                        m_TokenBoard.AddUTF8Code(uint32_t('C'));
                                    }else{
                                        m_TokenBoard.AddUTF8Code(uint32_t('c'));
                                    }
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
                                    if(false
                                            || rstEvent.key.keysym.mod & KMOD_LSHIFT
                                            || rstEvent.key.keysym.mod & KMOD_RSHIFT){
                                        m_TokenBoard.AddUTF8Code(uint32_t('V'));
                                    }else{
                                        m_TokenBoard.AddUTF8Code(uint32_t('v'));
                                    }
                                }
                                break;
                            }

                        default:
                            {
                                char chKeyName = SDLKeyEventChar(rstEvent);
                                if(chKeyName != '\0'){
                                    m_TokenBoard.AddUTF8Code((uint32_t)(chKeyName));
                                }
                                break;
                            }

                    }

                    ResetTokenBoardLocation();
                    return true;
                }
                break;
            }
        case SDL_TEXTINPUT:
            {
                std::string szXMLContent;

                szXMLContent += "<ROOT><OBJECT>";
                szXMLContent += rstEvent.text.text ? rstEvent.text.text : "";
                szXMLContent += "</OBJECT></ROOT>";

                m_TokenBoard.AppendXML(szXMLContent.c_str(), {});
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
    int nCursorX, nCursorY, nTBX, nTBY, nTBW, nTBH, nX, nY, nH;
    m_TokenBoard.GetCursor(&nCursorX, &nCursorY);

    if(m_TokenBoard.GetTokenBoxInfo(nCursorX - 1, nCursorY, nullptr, &nTBX, &nTBY, &nTBW, &nTBH, nullptr, nullptr)){
        nX = nTBX + nTBW;
        nY = nTBY - m_TokenBoard.GetLineSpace() / 2;
        nH = nTBH + m_TokenBoard.GetLineSpace();
    }else if(m_TokenBoard.GetTokenBoxInfo(nCursorX, nCursorY, nullptr, &nTBX, &nTBY, &nTBW, &nTBH, nullptr, nullptr)){
        nX = nTBX - m_CursorWidth;
        nY = nTBY - m_TokenBoard.GetLineSpace() / 2;
        nH = nTBH + m_TokenBoard.GetLineSpace();
    }else{
        // even both sides give me failure, still the board may be non-empty
        // i.e. when the cursor is at the beginning of an blank line
        // but this board also contains non-empty lines
        //
        int nStartY = m_TokenBoard.GetLineStartY(nCursorY);
        int nBlankLineH = m_TokenBoard.GetBlankLineHeight();

        nX = m_TokenBoard.Margin(3);
        nY = nStartY - nBlankLineH - m_TokenBoard.GetLineSpace() / 2;
        nH = nBlankLineH + m_TokenBoard.GetLineSpace();
    }

    if(pX){ *pX = nX;                         }
    if(pY){ *pY = nY;                         }
    if(pW){ *pW = std::max(1, m_CursorWidth); }
    if(pH){ *pH = nH;                         }
}

// we always move the cursor point into the visiable region
// in this model inputboard is a view window of the tokenboard
//
void InputBoard::ResetTokenBoardLocation()
{
    // cursor is a rectangle, and (nX, nY, nW, nH) are the coordinate
    // and size of it, with top-left originated on TokenBoard
    // 
    // +------------------------
    // | InputBoard
    // |
    // |   (0, 0)
    // |    +-------------------
    // |    | TokenBoard
    // |    |
    // |    |     (X, Y)
    // |    |     +-+
    // |    |     | |
    // |    |     | |
    int nX, nY, nW, nH;
    GetCursorInfo(&nX, &nY, &nW, &nH);

    // make (nX, nY) to be the coordinate w.r.t. InputBoard
    nX += m_TokenBoard.X();
    nY += m_TokenBoard.Y();

    // cursor is not contained in the view window, need to be reset
    if(!RectangleInside(0, 0, W(), H(), nX, nY, nW, nH)){
        if(nX < 0){
            m_TokenBoard.Move(-nX, 0);
        }

        if(nX + nW > W()){
            m_TokenBoard.Move(W() - (nX + nW), 0);
        }

        if(nY < 0){
            m_TokenBoard.Move(0, -nY);
        }

        if(nY + nH > H()){
            m_TokenBoard.Move(0, H() - (nY + nH));
        }
    }
}

void InputBoard::Draw()
{
    // m_TokenBoard and InputBoard are of the ``has-a" relationship
    // and coordinate of TokenBoard is relative to top-left of InputBoard
    int nTBDX = m_TokenBoard.X();
    int nTBDY = m_TokenBoard.Y();
    int nTBW  = m_TokenBoard.W();
    int nTBH  = m_TokenBoard.H();

    if(RectangleOverlapRegion(0, 0, W(), H(), &nTBDX, &nTBDY, &nTBW, &nTBH)){
        int nTBX = nTBDX - m_TokenBoard.X();
        int nTBY = nTBDY - m_TokenBoard.Y();
        m_TokenBoard.DrawEx(nTBDX + X(), nTBDY + Y(), nTBX, nTBY, nTBW, nTBH);
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
        extern SDLDevice *g_SDLDevice;
        g_SDLDevice->PushColor(m_CursorColor.r, m_CursorColor.g, m_CursorColor.b, m_CursorColor.a);
        g_SDLDevice->FillRectangle(X() + m_TokenBoard.X() + nX, Y() + m_TokenBoard.Y() + nY, nW, nH);
        g_SDLDevice->PopColor();
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

void InputBoard::DrawEx(
        int nDstX, int nDstY, // start position of drawing on the screen
        int nSrcX, int nSrcY, // region to draw, a cropped region on the token board
        int nSrcW, int nSrcH)
{
    m_TokenBoard.DrawEx(nDstX, nDstY, nSrcX, nSrcY, nSrcW, nSrcH);
}

std::string InputBoard::Content()
{
    XMLObjectList stList;
    stList.Parse(Print(false).c_str(), true);

    stList.Reset();
    auto pObject = stList.Fetch();

    return std::string(pObject ? pObject->GetText() : "");
}

void InputBoard::InsertInfo(const char *)
{
}
