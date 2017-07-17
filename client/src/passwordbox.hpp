#pragma once
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
