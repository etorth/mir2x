/*
 * =====================================================================================
 *
 *       Filename: inputline.hpp
 *        Created: 06/19/2017 11:05:07
 *  Last Modified: 06/19/2017 11:43:01
 *
 *    Description: for InputBoard I take TokenBoard as an internal component
 *                 because I think InputBoard is something to handle input/output
 *                 
 *                 but for InputLine I take it as a derived class of InptutBoard
 *                 I won't design it to contains an InputBoard inside
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
#include "inputboard.hpp"

class InputLine: public InputBoard
{
    public:
        InputLine(
                int              nX,
                int              nY,
                int              nW,
                int              nH,
                bool             bSpacing,
                int              nMaxWidth       = -1,
                int              nCursorWidth    =  2,
                const SDL_Color &rstCursorColor  = {0XFF, 0XFF, 0XFF, 0XFF},
                uint8_t          nDefaultFont    =  0,
                uint8_t          nDefaultSize    =  10,
                uint8_t          nDefaultStyle   =  0,
                const SDL_Color &rstDefaultColor = {0XFF, 0XFF, 0XFF, 0XFF},
                Widget          *pWidget         =  nullptr,
                bool             bFreeWidget     =  false)
            : InputBoard(nX,
                         nY,
                         nW,
                         nH,
                         bSpacing,
                         nMaxWidth,
                         0,
                         nCursorWidth,
                         rstCursorColor,
                         nDefaultFont,
                         nDefaultSize,
                         nDefaultStyle,
                         rstDefaultColor,
                         pWidget,
                         bFreeWidget)
        {}

       ~InputLine() = default;

    public:
        bool ProcessEvent(const SDL_Event &, bool *);
};
