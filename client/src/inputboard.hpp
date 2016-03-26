/*
 * =====================================================================================
 *
 *       Filename: inputboard.hpp
 *        Created: 06/17/2015 10:24:27 PM
 *  Last Modified: 03/26/2016 12:07:49
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
#include "tokenboard.hpp"

class InputBoard: public InputWidget
{
    public:
        InputBoard(int nX, int nY,
                int     nMaxWidth         = -1,
                int     nMinMarginBtwBox  = 0,
                int     nMinMarginBtwLine = 0,
                Widget *pWidget           = nullptr,
                bool    bFreeWidget       = false)
            : Widget(nX, nY, 0, 0, pWidget, bFreeWidget)
            , m_PW(nMaxWidth)
            , m_MinMarginBtwBox(nMinMarginBtwBox)
            , m_MinMarginBtwLine(nMinMarginBtwLine)
        {
            auto pTokenBoard = new TokenBoard(...);
            this->Add(pTokenBoard);
        }
        virtual ~InputBoard() = default;

    public:
        bool ProcessEvent(const SDL_Event &, bool *);

    protected:
        void SetProperH();
        void DrawCursor();
        void DrawSystemCursor();
        void PushBack(TOKENBOX &);
        void ResetShowStartX();
        void SetTokenBoxStartX();
        void BindCursorTokenBox(int, int);
        void LoadUTF8CharBoxCache(TOKENBOX &);

    protected:
        int      m_CursorWidth;
        uint32_t m_CursorColor;
        uint8_t  m_FontSet;
        uint8_t  m_Size;
        uint32_t m_TextColor;

    protected:
        int     m_SystemCursorX;
        int     m_SystemCursorY;
        bool    m_DrawOwnSystemCursor;
        int     m_BindTokenBoxIndex;
        int     m_ShowStartX;
        Uint32  m_Ticks;
        bool    m_Focus;

    protected:
        std::vector<TOKENBOX>       m_Line;
        std::string                 m_Content;
        TokenBoard                 *m_TokenBoard;

    public:
        static int  m_ShowSystemCursorCount;
        static int  m_InputBoxCount;

    private:
        IMEBase *m_IME;
};
