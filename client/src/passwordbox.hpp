#pragma once
#include "idbox.hpp"

class PasswordBox: public IDBox
{
    public:
        PasswordBox(
                int              nX,
                int              nY,
                int              nW,
                int              nH,
                bool             bSecurity      = true,
                int              nCursorWidth   = 2,
                uint8_t          nFont          = 0,
                uint8_t          nFontSize      = 18,
                const SDL_Color &rstFontColor   = {0XFF, 0XFF, 0XFF, 0XFF},
                const SDL_Color &rstCursorColor = {0XFF, 0XFF, 0XFF, 0XFF},
                Widget          *pWidget        = nullptr,
                bool             bFreeWidget    = false)
            : IDBox(
                    nX,
                    nY,
                    nW,
                    nH,
                    nCursorWidth,
                    nFont, nFontSize,
                    rstFontColor,
                    rstCursorColor,
                    pWidget,
                    bFreeWidget)
            , m_Security(bSecurity)
        {}

        ~PasswordBox() = default;

    private:
        bool m_Security;
};
