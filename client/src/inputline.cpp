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

#include "mathfunc.hpp"
#include "inputline.hpp"
#include "sdlkeychar.hpp"

bool inputLine::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return false;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_TAB:
                        {
                            if(focus() && m_tabCB){
                                m_tabCB();
                                return true;
                            }
                            return false;
                        }
                    case SDLK_RETURN:
                        {
                            if(focus() && m_returnCB){
                                m_returnCB();
                                return true;
                            }
                            return false;
                        }
                    default:
                        {
                            const char text[2]
                            {
                                sdlKeyChar(event), '\0',
                            };

                            m_tpset.insertUTF8String(m_cursor++, 0, text);
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

                const int eventX = X() - event.button.x;
                const int eventY = Y() - event.button.y;

                const auto [cursorX, cursorY] = m_tpset.locCursor(eventX, eventY);
                if(cursorY != 0){
                    throw fflerror("cursor locates at wrong line");
                }

                m_cursor = cursorY;

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
    const auto needDraw = MathFunc::ROICrop(
            &srcX, &srcY,
            &srcW, &srcH,
            &dstX, &dstY,

            W(),
            H(),

            m_tpsetX, m_tpsetY, m_tpset.PW(), m_tpset.PH());

    if(!needDraw){
        return;
    }
    m_tpset.drawEx(dstX, dstY, srcX - m_tpsetX, srcY - m_tpsetY, srcW, srcH);
}

std::string inputLine::getString() const
{
    return "12";
}
