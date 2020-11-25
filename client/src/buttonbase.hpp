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
    public:
        enum buttonState: int
        {
            BUTTON_OFF     = 0,
            BUTTON_OVER    = 1,
            BUTTON_PRESSED = 2,
        };

    protected:
        int m_state;

    protected:
        bool m_onClickDone;

    protected:
        int m_offset[3][2];

    protected:
        std::function<void()> m_onOver;
        std::function<void()> m_onClick;
        
    public:
        ButtonBase(
                int nX,
                int nY,
                int nW,
                int nH,

                const std::function<void()> &fnOnOver  = nullptr,
                const std::function<void()> &fnOnClick = nullptr,

                int nOffXOnOver  = 0,
                int nOffYOnOver  = 0,
                int nOffXOnClick = 0,
                int nOffYOnClick = 0,

                bool    bOnClickDone = true,
                Widget *pwidget      = nullptr,
                bool    bFreewidget  = false)
            : Widget(nX, nY, nW, nH, pwidget, bFreewidget)
            , m_state(BUTTON_OFF)
            , m_onClickDone(bOnClickDone)
            , m_offset
              {
                  {0            , 0           },
                  {nOffXOnOver  , nOffYOnOver },
                  {nOffXOnClick , nOffYOnClick},
              }
            , m_onOver (fnOnOver)
            , m_onClick(fnOnClick)
        {
            // we don't fail even if x, y, w, h are invalid
            // because derived class could reset it in its constructor
        }

    public:
        virtual ~ButtonBase() = default;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    protected:
        int OffX() const
        {
            return m_offset[State()][0];
        }

        int OffY() const
        {
            return m_offset[State()][1];
        }

        int State() const
        {
            return m_state;
        }

    public:
        // we can automatically do this or call this function
        // sometimes when we invoke the callback it changes the button location
        void setOff()
        {
            m_state = BUTTON_OFF;
        }
};
