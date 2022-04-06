/*
 * =====================================================================================
 *
 *       Filename: tritexbutton.hpp
 *        Created: 08/26/2016 13:20:23
 *    Description: button with three texture, it has a position shift when
 *                 state changes.
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
#include "buttonbase.hpp"

class TritexButton: public ButtonBase
{
    private:
        uint32_t m_texID[3];

    private:
        const bool m_alterColor;

    public:
        TritexButton(
                dir8_t argDir,
                int argX,
                int argY,

                const uint32_t (&texID)[3],

                std::function<void()> fnOnOverIn  = nullptr,
                std::function<void()> fnOnOverOut = nullptr,
                std::function<void()> fnOnClick   = nullptr,

                int offXOnOver  = 0,
                int offYOnOver  = 0,
                int offXOnClick = 0,
                int offYOnClick = 0,

                bool    onClickDone = true,
                bool    alterColor  = true,
                Widget *widgetPtr   = nullptr,
                bool    autoDelete  = false)
            : ButtonBase
              {
                  argDir,
                  argX,
                  argY,
                  0,
                  0,

                  std::move(fnOnOverIn),
                  std::move(fnOnOverOut),
                  std::move(fnOnClick),

                  SYS_U32NIL,
                  SYS_U32NIL,
                  SYS_U32NIL,

                  offXOnOver,
                  offYOnOver,
                  offXOnClick,
                  offYOnClick,

                  onClickDone,
                  widgetPtr,
                  autoDelete,
              }
            , m_texID
              {
                  texID[0],
                  texID[1],
                  texID[2],
              }
            , m_alterColor(alterColor)
        {
            // hide PNGTexDB and SDLDevice
            // query texture size and setup the button size
            initButtonSize();
        }

    public:
        void drawEx(int,                 // dst x on the screen coordinate
                    int,                 // dst y on the screen coordinate
                    int,                 // src x on the widget, take top-left as origin
                    int,                 // src y on the widget, take top-left as origin
                    int,                 // size to draw
                    int) const override; // size to draw
    private:
        void initButtonSize();

    public:
        void setTexID(const uint32_t (&texID)[3])
        {
            for(int i: {0, 1, 2}){
                m_texID[i] = texID[i];
            }
        }
};
