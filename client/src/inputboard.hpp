/*
 * =====================================================================================
 *
 *       Filename: inputboard.hpp
 *        Created: 06/17/2015 10:24:27 PM
 *  Last Modified: 03/31/2016 23:52:53
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
        InputBoard(
                int              nX,
                int              nY,
                bool             bSpacing,
                int              nMaxWidth       = -1,
                int              nWordSpace      =  0,
                int              nLineSpace      =  0,
                int              nCursorWidth    =  2,
                const SDL_Color &rstCursorColor  = {0XFF, 0XFF, 0XFF, 0XFF},
                uint8_t          nDefaultFont    =  0,
                uint8_t          nDefaultSize    =  0,
                uint8_t          nDefaultStyle   =  0,
                const SDL_Color &rstDefaultColor = {0XFF, 0XFF, 0XFF, 0XFF},
                Widget          *pWidget         =  nullptr,
                bool             bFreeWidget     =  false):
            InputWidget(nX, nY, 0, 0, pWidget, bFreeWidget)
            , m_TokenBoard(
                    0,
                    0,
                    true,
                    true,
                    bSpacing,
                    false,
                    nMaxWidth,
                    nWordSpace,
                    nLineSpace,
                    nDefaultFont,
                    nDefaultSize,
                    nDefaultStyle,
                    rstDefaultColor,
                    0,
                    nCursorWidth,
                    0,
                    nCursorWidth,
                    pWidget,
                    bFreeWidget)
            , m_CursorColor(rstCursorColor)
            , m_CursorWidth{
                nCursorWidth
            }
        {

            m_MS = 0.0;
            s_InputBoardCount++;
            s_ShowSystemCursorCount++;

            SetProperH();
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


        void GetCursorInfo(int *, int *, int *, int *);
        void ResetTokenBoardLoction();

        void Draw();

    protected:
        TokenBoard                  m_TokenBoard;
        SDL_Color m_CursorColor;
        int      m_CursorWidth;
        uint8_t  m_FontSet;
        uint8_t  m_Size;
        uint32_t m_TextColor;

    protected:
        int     m_SystemCursorX;
        int     m_SystemCursorY;
        bool    m_DrawOwnSystemCursor;
        int     m_BindTokenBoxIndex;
        int     m_ShowStartX;
        bool    m_Focus;
        double  m_MS;

    protected:
        std::vector<TOKENBOX>       m_Line;
        std::string                 m_Content;

    public:
        static int  s_ShowSystemCursorCount;
        static int  s_InputBoardCount;

    private:
        IMEBase *m_IME;
};
