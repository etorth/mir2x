/*
 * =====================================================================================
 *
 *       Filename: inputboard.cpp
 *        Created: 08/21/2015 07:04:16
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

#include "log.hpp"
#include "client.hpp"
#include "mathf.hpp"
#include "fontexdb.hpp"
#include "inputboard.hpp"
#include "sdlkeychar.hpp"

extern Log *g_log;
extern Client *g_client;
extern SDLDevice *g_SDLDevice;

int InputBoard::s_ShowSystemCursorCount = 0;
int InputBoard::s_InputBoardCount       = 0;

void InputBoard::update(double fMS)
{
    m_MS += fMS;
    Widget::update(fMS);
    m_tokenBoard.Update(fMS);
}

bool InputBoard::processEvent(const SDL_Event &event, bool valid)
{
    // here we parse all event
    // even some else widget have captured this event

    // for example, A and B are two inputboards
    // if we pressed in A, then B should lost its focus
    //
    // +-------+ +-------+
    // |   A   | |   B   |
    // +-------+ +-------+

    switch(event.type){
        case SDL_MOUSEMOTION:
            {
                if(in(event.motion.x, event.motion.y)){
                    m_systemCursorX = event.motion.x;
                    m_systemCursorY = event.motion.y;
                    if(!m_drawOwnSystemCursor){
                        s_ShowSystemCursorCount--;
                    }
                    m_drawOwnSystemCursor = true;
                }
                else{
                    if(m_drawOwnSystemCursor){
                        s_ShowSystemCursorCount++;
                    }
                    m_drawOwnSystemCursor = false;
                }
                return m_tokenBoard.processEvent(event, valid);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(in(event.button.x, event.button.y)){
                    m_tokenBoard.processEvent(event, true);

                    RelocateTokenBoard();
                    focus(true);
                    return true;
                }
                else{
                    focus(false);
                    return false;
                }
            }
        case SDL_KEYDOWN:
            {
                if(focus()){
                    // clear the count no matter what key pressed
                    m_MS = 0.0;
                    switch(event.key.keysym.sym){
                        case SDLK_UP:
                            {
                                int nX, nY;
                                m_tokenBoard.GetCursor(&nX, &nY);
                                if(!m_tokenBoard.SetCursor(nX, nY - 1)){
                                    m_tokenBoard.SetCursor(m_tokenBoard.GetLineTokenBoxCount(nY - 1), nY - 1);
                                }
                                break;
                            }
                        case SDLK_DOWN:
                            {
                                int nX, nY;
                                m_tokenBoard.GetCursor(&nX, &nY);
                                if(!m_tokenBoard.SetCursor(nX, nY + 1)){
                                    m_tokenBoard.SetCursor(m_tokenBoard.GetLineTokenBoxCount(nY + 1), nY + 1);
                                }
                                break;
                            }

                        case SDLK_LEFT:
                            {
                                int nX, nY;
                                m_tokenBoard.GetCursor(&nX, &nY);
                                if(!m_tokenBoard.SetCursor(nX - 1, nY)){
                                    if(nY > 0){
                                        nX = m_tokenBoard.GetLineTokenBoxCount(nY - 1);
                                        m_tokenBoard.SetCursor(nX, nY - 1);
                                    }
                                }
                                break;
                            }

                        case SDLK_RIGHT:
                            {
                                int nX, nY;
                                m_tokenBoard.GetCursor(&nX, &nY);
                                if(!m_tokenBoard.SetCursor(nX + 1, nY)){
                                    if(nY + 1 < m_tokenBoard.GetLineCount()){
                                        m_tokenBoard.SetCursor(0, nY + 1);
                                    }
                                }
                                break;
                            }

                        case SDLK_BACKSPACE:

                            {
                                m_tokenBoard.Delete(false);
                                break;
                            }

                        case SDLK_RETURN:
                            {
                                m_tokenBoard.BreakLine();
                                break;
                            }

                        case SDLK_ESCAPE:
                            {
                                focus(false);
                                break;
                            }

                        case SDLK_x:
                            {
                                if(false
                                        || event.key.keysym.mod & KMOD_LCTRL
                                        || event.key.keysym.mod & KMOD_RCTRL){
                                    g_client->Clipboard(m_tokenBoard.GetXML(true));
                                    m_tokenBoard.Delete(true);
                                }else{
                                    if(SDL_IsTextInputActive() == SDL_FALSE){
                                        if(false
                                                || event.key.keysym.mod & KMOD_LSHIFT
                                                || event.key.keysym.mod & KMOD_RSHIFT){
                                            m_tokenBoard.AddUTF8Code(uint32_t('X'));
                                        }else{
                                            m_tokenBoard.AddUTF8Code(uint32_t('x'));
                                        }
                                    }
                                }
                                break;
                            }

                        case SDLK_c:
                            {
                                if(false
                                        || event.key.keysym.mod & KMOD_LCTRL
                                        || event.key.keysym.mod & KMOD_RCTRL){
                                    g_client->Clipboard(m_tokenBoard.GetXML(true));
                                }else{
                                    if(SDL_IsTextInputActive() == SDL_FALSE){
                                        if(false
                                                || event.key.keysym.mod & KMOD_LSHIFT
                                                || event.key.keysym.mod & KMOD_RSHIFT){
                                            m_tokenBoard.AddUTF8Code(uint32_t('C'));
                                        }else{
                                            m_tokenBoard.AddUTF8Code(uint32_t('c'));
                                        }
                                    }
                                }
                                break;
                            }

                        case SDLK_v:
                            {
                                if(false
                                        || event.key.keysym.mod & KMOD_LCTRL
                                        || event.key.keysym.mod & KMOD_RCTRL){
                                    m_tokenBoard.ParseXML(g_client->Clipboard().c_str(), {});
                                }else{
                                    if(SDL_IsTextInputActive() == SDL_FALSE){
                                        if(false
                                                || event.key.keysym.mod & KMOD_LSHIFT
                                                || event.key.keysym.mod & KMOD_RSHIFT){
                                            m_tokenBoard.AddUTF8Code(uint32_t('V'));
                                        }else{
                                            m_tokenBoard.AddUTF8Code(uint32_t('v'));
                                        }
                                    }
                                }
                                break;
                            }

                        default:
                            {
                                if(SDL_IsTextInputActive() == SDL_FALSE){
                                    char chKeyName = sdlKeyChar(event);
                                    if(chKeyName != '\0'){
                                        m_tokenBoard.AddUTF8Code((uint32_t)(chKeyName));
                                    }
                                }
                                break;
                            }

                    }

                    RelocateTokenBoard();
                    return true;
                }
                break;
            }
        case SDL_TEXTINPUT:
            {
                if(focus()){
                    m_tokenBoard.AddUTF8Text(event.text.text);
                    RelocateTokenBoard();
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

void InputBoard::QueryCursor(int *pX, int *pY, int *pW, int *pH)
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
    int nCursorX = -1;
    int nCursorY = -1;
    m_tokenBoard.GetCursor(&nCursorX, &nCursorY);

    int nX = -1;
    int nY = -1;
    int nH = -1;

    int nTBX = -1;
    int nTBY = -1;
    int nTBW = -1;
    int nTBH = -1;

    if(m_tokenBoard.QueryTokenBox(nCursorX - 1, nCursorY, nullptr, &nTBX, &nTBY, &nTBW, &nTBH, nullptr, nullptr)){
        nX = nTBX + nTBW;
        nY = nTBY - m_tokenBoard.GetLineSpace() / 2;
        nH = nTBH + m_tokenBoard.GetLineSpace();
    }else if(m_tokenBoard.QueryTokenBox(nCursorX, nCursorY, nullptr, &nTBX, &nTBY, &nTBW, &nTBH, nullptr, nullptr)){
        nX = nTBX - m_cursorWidth;
        nY = nTBY - m_tokenBoard.GetLineSpace() / 2;
        nH = nTBH + m_tokenBoard.GetLineSpace();
    }else{
        // even both sides give me failure, still the board may be non-empty
        // i.e. when the cursor is at the beginning of an blank line
        // but this board also contains non-empty lines
        //
        int nStartY = m_tokenBoard.GetLineStartY(nCursorY);
        int nBlankLineH = m_tokenBoard.GetBlankLineHeight();

        nX = m_tokenBoard.Margin(3);
        nY = nStartY - nBlankLineH - m_tokenBoard.GetLineSpace() / 2;
        nH = nBlankLineH + m_tokenBoard.GetLineSpace();
    }

    if(pX){ *pX = nX;                                }
    if(pY){ *pY = nY;                                }
    if(pW){ *pW = (std::max<int>)(1, m_cursorWidth); }
    if(pH){ *pH = nH;                                }
}

// we always move the cursor point into the visiable region
// in this model inputboard is a view window of the tokenboard
//
void InputBoard::RelocateTokenBoard()
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
    QueryCursor(&nX, &nY, &nW, &nH);

    // make (nX, nY) to be the coordinate w.r.t. InputBoard
    nX += m_tokenBoard.x();
    nY += m_tokenBoard.y();

    if(nX < 0){
        m_tokenBoard.moveBy(-nX, 0);
    }

    if((nX + nW > w()) && (nW <= w())){
        m_tokenBoard.moveBy(w() - (nX + nW), 0);
    }

    if(nY < 0){
        m_tokenBoard.moveBy(0, -nY);
    }

    if((nY + nH > h()) && (nH <= h())){
        m_tokenBoard.moveBy(0, h() - (nY + nH));
    }
}

void InputBoard::Draw()
{
    // m_tokenBoard and InputBoard are of the ``has-a" relationship
    // and coordinate of TokenBoard is relative to top-left of InputBoard
    int nTBDX = m_tokenBoard.x();
    int nTBDY = m_tokenBoard.y();
    int nTBW  = m_tokenBoard.w();
    int nTBH  = m_tokenBoard.h();

    if(mathf::rectangleOverlapRegion(0, 0, w(), h(), &nTBDX, &nTBDY, &nTBW, &nTBH)){
        int nTBX = nTBDX - m_tokenBoard.x();
        int nTBY = nTBDY - m_tokenBoard.y();
        m_tokenBoard.drawEx(nTBDX + x(), nTBDY + y(), nTBX, nTBY, nTBW, nTBH);
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
    QueryCursor(&nX, &nY, &nW, &nH);

    if(((int)m_MS % 1000) < 500 && focus()){
        g_SDLDevice->PushColor(m_cursorColor.r, m_cursorColor.g, m_cursorColor.b, m_cursorColor.a);
        g_SDLDevice->FillRectangle(x() + m_tokenBoard.x() + nX, y() + m_tokenBoard.y() + nY, nW, nH);
        g_SDLDevice->PopColor();
    }

    // 2. draw ``I" cursor
    if(m_drawOwnSystemCursor){
        g_SDLDevice->PushColor(200, 200, 200, 200);
        g_SDLDevice->DrawLine(
                m_systemCursorX, m_systemCursorY - m_h / 2,
                m_systemCursorX, m_systemCursorY + m_h / 2);
        g_SDLDevice->DrawLine(
                m_systemCursorX - (std::max<int>)(1, (int)std::lround(m_h * 0.08)),
                m_systemCursorY - m_h / 2,
                m_systemCursorX + (std::max<int>)(1, (int)std::lround(m_h * 0.08)),
                m_systemCursorY - m_h / 2);
        g_SDLDevice->DrawLine(
                m_systemCursorX - (std::max<int>)(1, (int)std::lround(m_h * 0.08)),
                m_systemCursorY + m_h / 2,
                m_systemCursorX + (std::max<int>)(1, (int)std::lround(m_h * 0.08)),
                m_systemCursorY + m_h / 2);
        g_SDLDevice->PopColor();
    }

    if(s_ShowSystemCursorCount < s_InputBoardCount){
        SDL_ShowCursor(0);
    }else{
        SDL_ShowCursor(1);
    }
}

void InputBoard::drawEx(
        int nDstX, int nDstY, // start position of drawing on the screen
        int nSrcX, int nSrcY, // region to draw, a cropped region on the token board
        int nSrcW, int nSrcH)
{
    m_tokenBoard.drawEx(nDstX, nDstY, nSrcX, nSrcY, nSrcW, nSrcH);
}

std::string InputBoard::Content()
{
    std::string szContent;

    XMLObjectList stList;
    stList.Parse(Print(false).c_str(), true);

    for(auto pObject = stList.FirstElement(); pObject; pObject = pObject->NextSiblingElement()){
        switch(XMLObject::ObjectType(*pObject)){
            case OBJECTTYPE_RETURN:
                {
                    break;
                }
            case OBJECTTYPE_EMOTICON:
                {
                    break;
                }
            case OBJECTTYPE_EVENTTEXT:
            case OBJECTTYPE_PLAINTEXT:
                {
                    szContent += pObject->GetText();
                    break;
                }
            default:
                {
                    g_log->addLog(LOGTYPE_WARNING, "Detected known object type, ignored it");
                    break;
                }
        }
    }
    return szContent;
}
