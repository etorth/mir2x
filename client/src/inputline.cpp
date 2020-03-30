/*
 * =====================================================================================
 *
 *       Filename: inputline.cpp
 *        Created: 06/19/2017 11:29:06
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

#include <cmath>
#include "mathfunc.hpp"
#include "inputline.hpp"
#include "sdldevice.hpp"
#include "sdlkeychar.hpp"

extern SDLDevice *g_SDLDevice;

bool inputLine::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return false;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(!focus()){
                    return false;
                }

                switch(event.key.keysym.sym){
                    case SDLK_TAB:
                        {
                            if(m_tabCB){
                                m_tabCB();
                            }
                            return true;
                        }
                    case SDLK_RETURN:
                        {
                            if(m_returnCB){
                                m_returnCB();
                            }
                            return true;
                        }
                    case SDLK_LEFT:
                        {
                            m_cursor = std::max<int>(0, m_cursor - 1);
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_RIGHT:
                        {
                            if(m_tpset.empty()){
                                m_cursor = 0;
                            }
                            else{
                                m_cursor = std::min<int>(m_tpset.lineTokenCount(0), m_cursor + 1);
                            }
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_BACKSPACE:
                        {
                            if(m_cursor > 0){
                                m_tpset.deleteToken(m_cursor - 1, 0, 1);
                                m_cursor--;
                            }
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    default:
                        {
                            const char keyChar = sdlKeyChar(event);
                            if(keyChar != '\0'){
                                const char text[] {keyChar, '\0'};
                                m_tpset.insertUTF8String(m_cursor++, 0, text);
                            }
                            m_cursorBlink = 0.0;
                            return true;
                        }
                }
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(!in(event.button.x, event.button.y)){
                    focus(false);
                    return false;
                }

                const int eventX = event.button.x - X();
                const int eventY = event.button.y - Y();

                const auto [cursorX, cursorY] = m_tpset.locCursor(eventX, eventY);
                if(cursorY != 0){
                    throw fflerror("cursor locates at wrong line");
                }

                m_cursor = cursorX;
                focus(true);
                return true;
            }
        default:
            {
                return false;
            }
    }
}

void inputLine::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH)
{
    int srcCropX = srcX;
    int srcCropY = srcY;
    int srcCropW = srcW;
    int srcCropH = srcH;
    int dstCropX = dstX;
    int dstCropY = dstY;

    const auto needDraw = MathFunc::ROICrop(
            &srcCropX, &srcCropY,
            &srcCropW, &srcCropH,
            &dstCropX, &dstCropY,

            W(),
            H(),

            m_tpsetX, m_tpsetY, m_tpset.pw(), m_tpset.ph());

    if(!needDraw){
        return;
    }
    m_tpset.drawEx(dstCropX, dstCropY, srcCropX - m_tpsetX, srcCropY - m_tpsetY, srcCropW, srcCropH);

    if(std::fmod(m_cursorBlink, 1000.0) > 500.0){
        return;
    }

    if(!focus()){
        return;
    }

    int cursorY = Y() + m_tpsetY;
    int cursorX = X() + m_tpsetX + [this]()
    {
        if(m_tpset.empty() || m_cursor == 0){
            return 0;
        }

        if(m_cursor == m_tpset.lineTokenCount(0)){
            return m_tpset.pw();
        }

        const auto pToken = m_tpset.getToken(m_cursor - 1, 0);
        return pToken->Box.State.W1 + pToken->Box.State.X + pToken->Box.Info.W;
    }();

    int cursorW = m_cursorWidth;
    int cursorH = m_tpset.ph();

    if(MathFunc::RectangleOverlapRegion(dstX, dstY, srcW, srcH, &cursorX, &cursorY, &cursorW, &cursorH)){
        g_SDLDevice->FillRectangle(m_cursorColor + 128, cursorX, cursorY, cursorW, cursorH);
    }
}

std::string inputLine::getString() const
{
    return "12";
}
