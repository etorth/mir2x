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
#include <array>
#include "buttonbase.hpp"

class TritexButton: public ButtonBase
{
    private:
        int m_Offset[3][2];

    public:
        TritexButton(
                int                          nX,
                int                          nY,
                std::array<uint32_t, 3>      stvTexID,
                int                          nOffsetXOnOver  = 0,
                int                          nOffsetYOnOver  = 0,
                int                          nOffsetXOnClick = 0,
                int                          nOffsetYOnClick = 0,
                const std::function<void()> &fnOnOver        = [](){},
                const std::function<void()> &fnOnClick       = [](){},
                bool                         bOnClickDone    = true,
                Widget                      *pWidget         = nullptr,
                bool                         bFreeWidget     = false)
            : ButtonBase(
                    nX, 
                    nY,
                    stvTexID[0],
                    stvTexID[1],
                    stvTexID[2],
                    fnOnOver,
                    fnOnClick,
                    bOnClickDone,
                    pWidget,
                    bFreeWidget)
            , m_Offset {{0, 0}, {nOffsetXOnOver, nOffsetYOnOver}, {nOffsetXOnClick, nOffsetYOnClick}}
        {}

        // use one specified texture ID
        TritexButton(
                int                          nX,
                int                          nY,
                uint32_t                     nBaseTexID,
                int                          nOffsetXOnOver  = 0,
                int                          nOffsetYOnOver  = 0,
                int                          nOffsetXOnClick = 0,
                int                          nOffsetYOnClick = 0,
                const std::function<void()> &fnOnOver        = [](){},
                const std::function<void()> &fnOnClick       = [](){},
                bool                         bOnClickDone    = true,
                Widget                      *pWidget         = nullptr,
                bool                         bFreeWidget     = false)
            : TritexButton(
                    nX,
                    nY,
                    {nBaseTexID + 0, nBaseTexID + 1, nBaseTexID + 2},
                    nOffsetXOnOver,
                    nOffsetYOnOver,
                    nOffsetXOnClick,
                    nOffsetYOnClick,
                    fnOnOver,
                    fnOnClick,
                    bOnClickDone,
                    pWidget,
                    bFreeWidget)
        {}

        // use one specified texture ID
        // use default offset when event issued
        TritexButton(
                int                          nX,
                int                          nY,
                uint32_t                     nBaseTexID,
                const std::function<void()> &fnOnClick       = [](){},
                bool                         bOnClickDone    = true,
                Widget                      *pWidget         = nullptr,
                bool                         bFreeWidget     = false)
            : TritexButton(
                    nX,
                    nY,
                    nBaseTexID,
                    1,
                    1,
                    2,
                    2,
                    [](){},
                    fnOnClick,
                    bOnClickDone,
                    pWidget,
                    bFreeWidget)
        {}

    public:
        void DrawEx(int,    // dst x on the screen coordinate
                int,        // dst y on the screen coordinate
                int,        // src x on the widget, take top-left as origin
                int,        // src y on the widget, take top-left as origin
                int,        // size to draw
                int);       // size to draw
};
