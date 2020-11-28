/*
 * =====================================================================================
 *
 *       Filename: levelbox.cpp
 *        Created: 03/28/2020 05:47:00
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

#include "levelbox.hpp"
#include "sdldevice.hpp"

LevelBox::LevelBox(
        int x,
        int y,

        const std::function<void(int)> &onDrag,
        const std::function<void(   )> &onDoubleClick,

        Widget *parent,
        bool autoDelete)
    : Widget(x, y, 0, 0, parent, autoDelete)
    , m_label
      {
          0,
          0,
          u8"",
          0,
          12,
          0,
          colorf::YELLOW + 128,
          this,
          false,
      }
    , m_onDrag(onDrag)
    , m_onDoubleClick(onDoubleClick)
{
    setLevel(0);
}

bool LevelBox::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return false;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(!in(event.button.x, event.button.y)){
                    m_state = BEVENT_OFF;
                    return false;
                }

                if(event.button.clicks == 2){
                    m_onDoubleClick();
                }
                m_state = BEVENT_DOWN;
                return true;
            }
        case SDL_MOUSEBUTTONUP:
            {
                if(!in(event.button.x, event.button.y)){
                    m_state = BEVENT_OFF;
                    return false;
                }

                m_state = BEVENT_ON;
                return true;
            }
        case SDL_MOUSEMOTION:
            {
                // even not in the box
                // we still need to drag the widget

                if(m_state == BEVENT_DOWN){
                    if(event.motion.state & SDL_BUTTON_LMASK){
                        m_onDrag(event.motion.yrel);
                        return true;
                    }
                    else{
                        if(in(event.motion.x, event.motion.y)){
                            m_state = BEVENT_ON;
                            return true;
                        }
                        else{
                            m_state = BEVENT_OFF;
                            return false;
                        }
                    }
                }

                if(in(event.motion.x, event.motion.y)){
                    m_state = BEVENT_ON;
                    return true;
                }
                else{
                    m_state = BEVENT_OFF;
                    return false;
                }
            }
        default:
            {
                return false;
            }
    }
}

void LevelBox::drawEx(int dstX, int dstY, int, int, int, int)
{
    // don't worry too much here
    // we always draw fully for LevelBox

    const auto fnDrawCover = [dstX, dstY](uint32_t color)
    {
        if(auto *texPtr = g_SDLDevice->getCover(8)){
            SDL_SetTextureColorMod(texPtr, colorf::R(color), colorf::G(color), colorf::B(color));
            SDLDevice::EnableDrawBlendMode enableDrawBlendMode(SDL_BLENDMODE_BLEND);
            g_SDLDevice->drawTexture(texPtr, dstX + 1, dstY);
        }
    };

    switch(m_state){
        case BEVENT_ON:
            {
                fnDrawCover(colorf::RED);
                break;
            }
        case BEVENT_DOWN:
            {
                fnDrawCover(colorf::BLUE);
                break;
            }
        default:
            {
                break;
            }
    }

    const int dx = (w() - m_label.w()) / 2;
    const int dy = (h() - m_label.h()) / 2;

    m_label.drawEx(dstX + dx, dstY + dy, 0, 0, m_label.w(), m_label.h());
}
