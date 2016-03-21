/*
 * =====================================================================================
 *
 *       Filename: label.hpp
 *        Created: 08/20/2015 08:59:11 PM
 *  Last Modified: 03/20/2016 19:19:59
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
#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include "widget.hpp"
#include "tokenbox.hpp"
#include "tokenboard.hpp"
#include <cstdint>

class Label: public Widget
{
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
