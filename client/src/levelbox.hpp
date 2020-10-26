/*
 * =====================================================================================
 *
 *       Filename: levelbox.hpp
 *        Created: 03/28/2020 05:43:45
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

#pragma once
#include <functional>
#include "widget.hpp"
#include "bevent.hpp"
#include "sdldevice.hpp"
#include "colorf.hpp"
#include "labelboard.hpp"

extern SDLDevice *g_SDLDevice;

class LevelBox: public Widget
{
    private:
        LabelBoard m_label;

    private:
        int m_state = BEVENT_OFF;

    private:
        std::function<void(int)> m_onDrag;
        std::function<void()> m_onDoubleClick;

    public:
        LevelBox(
                int, // x
                int, // y

                const std::function<void(int)> &, // drag
                const std::function<void(   )> &, // double-click
                
                Widget * = nullptr, // parent
                bool     = false);  // auto-delete

    public:
        bool processEvent(const SDL_Event &, bool);

    public:
        void drawEx(int, int, int, int, int, int) override;

    public:
        void setLevel(int level)
        {
            m_label.setText(u8"%d", level);

            m_w = std::max<int>(m_label.w(), 18);
            m_h = std::max<int>(m_label.h(), 10);

            m_label.moveTo((w() - m_label.w()) / 2,(h() - m_label.h()) / 2);
        }
};
