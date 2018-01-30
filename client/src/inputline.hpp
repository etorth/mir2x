/*
 * =====================================================================================
 *
 *       Filename: inputline.hpp
 *        Created: 06/19/2017 11:05:07
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
#include <functional>
#include "colorfunc.hpp"
#include "inputboard.hpp"

class InputLine: public InputBoard
{
    protected:
        std::function<void()> m_TabFunc;
        std::function<void()> m_ReturnFunc;

    public:
        InputLine(
                int                     nX,
                int                     nY,
                int                     nW,
                int                     nH,
                int                     nCursorWidth    = 2,
                const SDL_Color        &rstCursorColor  = ColorFunc::COLOR_WHITE,
                uint8_t                 nDefaultFont    = 0,
                uint8_t                 nDefaultSize    = 10,
                uint8_t                 nDefaultStyle   = 0,
                const SDL_Color        &rstDefaultColor = ColorFunc::COLOR_WHITE,
                std::function<void()>   fnOnTab         = [](){},
                std::function<void()>   fnOnReturn      = [](){},
                Widget                 *pWidget         = nullptr,
                bool                    bFreeWidget     = false)
            : InputBoard(nX,
                         nY,
                         nW,
                         nH,
                         false,
                         nW,
                         0,
                         nCursorWidth,
                         rstCursorColor,
                         nDefaultFont,
                         nDefaultSize,
                         nDefaultStyle,
                         rstDefaultColor,
                         pWidget,
                         bFreeWidget)
            , m_TabFunc(fnOnTab)
            , m_ReturnFunc(fnOnReturn)
        {}

       ~InputLine() = default;

    public:
        bool ProcessEvent(const SDL_Event &, bool *);
};
