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

class ProcessRun;
class LevelBox: public Widget
{
    private:
        ProcessRun *m_processRun;

    private:
        int m_state = BEVENT_OFF;

    private:
        std::function<void(int)> m_onDrag;
        std::function<void(   )> m_onDoubleClick;

    public:
        LevelBox(
                dir8_t dir,
                int,
                int,

                ProcessRun *,
                const std::function<void(int)> &, // drag
                const std::function<void(   )> &, // double-click

                Widget * = nullptr, // parent
                bool     = false);  // auto-delete

    public:
        bool processEvent(const SDL_Event &, bool);

    public:
        void drawEx(int, int, int, int, int, int) const override;
};
