/*
 * =====================================================================================
 *
 *       Filename: idbox.hpp
 *        Created: 06/17/2015 10:24:27 PM
 *  Last Modified: 04/01/2016 23:39:36
 *
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

#include <vector>
#include <string>
#include <SDL2/SDL.h>

#include "widget.hpp"
#include "tokenbox.hpp"
#include "imebase.hpp"
#include "inputboard.hpp"

class IDBox: public InputBoard
{
    public:
        IDBox(
                int              nX,
                int              nY,
                int              nW,
                int              nH,
                int              nCursorWidth   = 2,
                uint8_t          nFont          = 0,
                uint8_t          nFontSize      = 18,
                const SDL_Color &rstFontColor   = {0XFF, 0XFF, 0XFF, 0XFF},
                const SDL_Color &rstCursorColor = {0XFF, 0XFF, 0XFF, 0XFF},
                Widget          *pWidget        = nullptr,
                bool             bFreeWidget    = false)
            : InputBoard(
                    nX,
                    nY,
                    nW,
                    nH,
                    false,
                   -1,
                    0,
                    nCursorWidth,
                    rstCursorColor,
                    nFont,
                    nFontSize,
                    0,
                    rstFontColor,
                    pWidget,
                    bFreeWidget)
            {}

        virtual ~IDBox() = default;
};
