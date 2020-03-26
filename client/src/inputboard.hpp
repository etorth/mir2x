/*
 * =====================================================================================
 *
 *       Filename: inputboard.hpp
 *        Created: 06/17/2015 10:24:27
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
#include "tokenboard.hpp"

class InputBoard: public Widget
{
    protected:
        TokenBoard m_TokenBoard;
        SDL_Color  m_CursorColor;
        int        m_CursorWidth;
        uint8_t    m_FontSet;
        uint8_t    m_Size;
        uint32_t   m_TextColor;

    protected:
        int     m_SystemCursorX;
        int     m_SystemCursorY;
        bool    m_DrawOwnSystemCursor;
        int     m_BindTokenBoxIndex;
        int     m_ShowStartX;
        double  m_MS;

    public:
        static int  s_ShowSystemCursorCount;
        static int  s_InputBoardCount;

    public:
        InputBoard(
                int              nX,
                int              nY,
                int              nW,
                int              nH,
                bool             bSpacing,
                int              nMaxWidth       = -1,
                int              nLineSpace      =  0,
                int              nCursorWidth    =  2,
                const SDL_Color &rstCursorColor  = {0XFF, 0XFF, 0XFF, 0XFF},
                uint8_t          nDefaultFont    =  0,
                uint8_t          nDefaultSize    =  10,
                uint8_t          nDefaultStyle   =  0,
                const SDL_Color &rstDefaultColor = {0XFF, 0XFF, 0XFF, 0XFF},
                Widget          *pWidget         =  nullptr,
                bool             bFreeWidget     =  false)
            : Widget(nX, nY, nW, nH, pWidget, bFreeWidget)
            , m_TokenBoard(
                    0,
                    0,
                    true,
                    true,
                    bSpacing,
                    false,
                    nMaxWidth,
                    nCursorWidth,
                    nLineSpace,
                    nDefaultFont,
                    nDefaultSize,
                    nDefaultStyle,
                    rstDefaultColor,
                    0,
                    nCursorWidth,
                    0,
                    nCursorWidth,
                    nullptr,
                    false)
            , m_CursorColor(rstCursorColor)
            , m_CursorWidth(nCursorWidth)
        {

            m_MS = 0.0;
            s_InputBoardCount++;
            s_ShowSystemCursorCount++;
        }

        virtual ~InputBoard()
        {
            s_InputBoardCount--;
            s_ShowSystemCursorCount--;
            SDL_ShowCursor(1);
        }

    public:
        void Update(double);

    public:
        std::string Print(bool bSelectedOnly)
        {
            return m_TokenBoard.Print(bSelectedOnly);
        }

    public:
        bool ProcessEvent(const SDL_Event &, bool *);

    public:
        void QueryCursor(int *, int *, int *, int *);
        void RelocateTokenBoard();

    public:
        void Draw();
        void drawEx(int, int, int, int, int, int);

    public:
        std::string Content();
};
