/*
 * =====================================================================================
 *
 *       Filename: label.hpp
 *        Created: 08/20/2015 08:59:11 PM
 *  Last Modified: 04/01/2016 13:30:37
 *
 *    Description: Label is a class
 *                      1. without padding
 *                      2. without wrapping
 *                      3. non-editable
 *                      4. non-selectable
 *                      5. won't accept any events
 *                      6. only have one type of font
 *                      7. may contain emoticons
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
#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include "widget.hpp"
#include "tokenbox.hpp"
#include "tokenboard.hpp"
#include <cstdint>

class Label: public Widget
{
    public:
        Label(
                int              nX,
                int              nY,
                const char      *szContent   = "",
                uint8_t          nFont       = 0,
                uint8_t          nSize       = 10,
                uint8_t          nStyle      = 0,
                const SDL_Color &rstColor    = {0XFF, 0XFF, 0XFF, 0XFF},
                Widget          *pWidget     = nullptr,
                bool             bFreeWidget = false):
            Widget(nX, nY, 0, 0, pWidget, bFreeWidget)
            , m_FontKey((uint32_t(nFont) << 16) + (uint32_t(nSize) << 8) + nStyle)
            , m_Color(rstColor)
            , m_Content(szContent)
            , m_TokenBoard {
                0,
                0,
                false,
                false,
                false,
                false,
                -1,
                0,
                0,
                nFont,
                nSize,
                nStyle,
                rstColor,
                0,
                0,
                0,
                0,
                nullptr,
                false
            }
        {
            SetText(szContent);
        }


    private:
        uint32_t    m_FontKey;
        SDL_Color   m_Color;
        std::string m_Content;
        TokenBoard  m_TokenBoard;

    public:
        Label(
                uint8_t,           // font index
                uint8_t,           // font size
                uint8_t,           // text style
                const SDL_Color &, // text color
                const char *);     // text content

        ~Label() = default;

    public:
        int W()
        {
            return m_TokenBoard.W();
        }

        int H()
        {
            return m_TokenBoard.H();
        }

        const char *Text()
        {
            return m_Content.c_str();
        }

        void SetText(const char *);

        void Draw(int nX, int nY)
        {
            m_TokenBoard.Draw(nX, nY);
        }
};
