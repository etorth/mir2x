/*
 * =====================================================================================
 *
 *       Filename: idbox.hpp
 *        Created: 06/17/2015 10:24:27 PM
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
#include <functional>

#include "widget.hpp"
#include "tokenbox.hpp"
#include "inputboard.hpp"

class IDBox: public InputBoard
{
    private:
        std::function<void()> m_OnTab;
        std::function<void()> m_OnEnter;

    public:
        IDBox(
                int                          nX,
                int                          nY,
                int                          nW,
                int                          nH,
                int                          nCursorWidth   = 2,
                uint8_t                      nFont          = 0,
                uint8_t                      nFontSize      = 18,
                const SDL_Color             &rstFontColor   = {0XFF, 0XFF, 0XFF, 0XFF},
                const SDL_Color             &rstCursorColor = {0XFF, 0XFF, 0XFF, 0XFF},
                const std::function<void()> &fnOnTab        = [](){},
                const std::function<void()> &fnOnEnter      = [](){},
                Widget                      *pWidget        = nullptr,
                bool                         bFreeWidget    = false)
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
            , m_OnTab(fnOnTab)
            , m_OnEnter(fnOnEnter)
        {}

    public:
        virtual ~IDBox() = default;

    public:
        bool processEvent(const SDL_Event &, bool *);
};
