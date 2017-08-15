/*
 * =====================================================================================
 *
 *       Filename: passwordbox.hpp
 *        Created: 07/16/2017 19:06:25
 *  Last Modified: 08/14/2017 15:44:07
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
#include <string>
#include "idbox.hpp"

class PasswordBox: public IDBox
{
    private:
        bool m_Security;

    public:
        PasswordBox(
                int                          nX,
                int                          nY,
                int                          nW,
                int                          nH,
                bool                         bSecurity      = true,
                int                          nCursorWidth   = 2,
                uint8_t                      nFont          = 0,
                uint8_t                      nFontSize      = 18,
                const SDL_Color             &rstFontColor   = {0XFF, 0XFF, 0XFF, 0XFF},
                const SDL_Color             &rstCursorColor = {0XFF, 0XFF, 0XFF, 0XFF},
                const std::function<void()> &fnOnTab        = [](){},
                const std::function<void()> &fnOnEnter      = [](){},
                Widget                      *pWidget        = nullptr,
                bool                         bFreeWidget    = false)
            : IDBox(
                    nX,
                    nY,
                    nW,
                    nH,
                    nCursorWidth,
                    nFont,
                    nFontSize,
                    rstFontColor,
                    rstCursorColor,
                    fnOnTab,
                    fnOnEnter,
                    pWidget,
                    bFreeWidget)
            , m_Security(bSecurity)
        {}

    public:
        ~PasswordBox() = default;
};
