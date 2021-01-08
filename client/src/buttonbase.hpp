/*
 * =====================================================================================
 *
 *       Filename: buttonbase.hpp
 *        Created: 08/25/2016 04:12:57
 *    Description:
 *
 *              basic button class to handle event logic only
 *              1. no draw
 *              2. no texture id field
 *
 *              I support two callbacks only: off->on and on->click
 *              this class ask user to configure whether the on->click is triggered
 *              at the PRESS or RELEASE event.
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
#include <cstdint>
#include <functional>

#include "widget.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"

class ButtonBase: public Widget
{
    private:
        int m_state     = BEVENT_OFF;
        int m_lastState = BEVENT_OFF;

    protected:
        const bool m_onClickDone;

    protected:
        const int m_offset[3][2];

    protected:
        std::function<void()> m_onOverIn;
        std::function<void()> m_onOverOut;
        std::function<void()> m_onClick;

    public:
        ButtonBase(
                int argX,
                int argY,
                int argW,
                int argH,

                const std::function<void()> &fnOnOverIn  = nullptr,
                const std::function<void()> &fnOnOverOut = nullptr,
                const std::function<void()> &fnOnClick   = nullptr,

                int offXOnOver  = 0,
                int offYOnOver  = 0,
                int offXOnClick = 0,
                int offYOnClick = 0,

                bool    onClickDone = true,
                Widget *widgetPtr   = nullptr,
                bool    autoFree    = false)
            : Widget(argX, argY, argW, argH, widgetPtr, autoFree)
            , m_onClickDone(onClickDone)
            , m_offset
              {
                  {0            , 0          },
                  {offXOnOver   , offYOnOver },
                  {offXOnClick  , offYOnClick},
              }
            , m_onOverIn(fnOnOverIn)
            , m_onOverOut(fnOnOverOut)
            , m_onClick(fnOnClick)
        {
            // we don't fail even if x, y, w, h are invalid
            // because derived class could do reset in its constructor
        }

    public:
        virtual ~ButtonBase() = default;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    protected:
        int offX() const
        {
            return m_offset[getState()][0];
        }

        int offY() const
        {
            return m_offset[getState()][1];
        }

    public:
        int getState() const
        {
            return m_state;
        }

        int getLastState() const
        {
            return m_lastState;
        }

    public:
        void setState(int state)
        {
            m_lastState = m_state;
            m_state = state;
        }

    public:
        // we can automatically do this or call this function
        // sometimes when we invoke the callback it changes the button location
        void setOff()
        {
            setState(BEVENT_OFF);
        }
};
