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
#include "colorfunc.hpp"
#include "labelboard.hpp"

extern SDLDevice *g_SDLDevice;

class levelBox: public widget
{
    private:
        labelBoard m_label;

    private:
        int m_state = BEVENT_OFF;

    private:
        SDL_Texture * const m_texture = nullptr;

    private:
        std::function<void(int)> m_onDrag;
        std::function<void()> m_onDoubleClick;

    public:
        levelBox(int, int, std::function<void(int)>, std::function<void()>, widget *, bool);

    public:
        bool processEvent(const SDL_Event &, bool);

    public:
        void drawEx(int, int, int, int, int, int) override;

    public:
        void setLevel(int level)
        {
            m_label.setText("%d", level);

            m_W = std::max<int>(m_label.W(), 18);
            m_H = std::max<int>(m_label.H(), 10);

            m_label.moveTo((W() - m_label.W()) / 2,(H() - m_label.H()) / 2);
        }
};
