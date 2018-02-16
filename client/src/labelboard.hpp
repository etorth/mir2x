/*
 * =====================================================================================
 *
 *       Filename: labelboard.hpp
 *        Created: 08/20/2015 08:59:11
 *    Description:
 *
 *              LabelBoard is a class with features:
 *
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
#include <vector>
#include <string>
#include <cstdint>
#include <SDL2/SDL.h>

#include "widget.hpp"
#include "tokenbox.hpp"
#include "tokenboard.hpp"

class LabelBoard: public Widget
{
    private:
        uint8_t   m_Font;
        uint8_t   m_FontSize;
        uint8_t   m_FontStyle;
        SDL_Color m_FontColor;

    private:
        std::string m_Content;
        TokenBoard  m_TokenBoard;

    public:
        LabelBoard(
                int              nX,
                int              nY,
                const char      *szContent   = "",
                uint8_t          nFont       = 0,
                uint8_t          nSize       = 10,
                uint8_t          nStyle      = 0,
                const SDL_Color &rstColor    = {0XFF, 0XFF, 0XFF, 0XFF},
                Widget          *pWidget     = nullptr,
                bool             bAutoDelete = false)
            : Widget(nX, nY, 0, 0, pWidget, bAutoDelete)
            , m_Font(nFont)
            , m_FontSize(nSize)
            , m_FontStyle(nStyle)
            , m_FontColor(rstColor)
            , m_Content(szContent)
            , m_TokenBoard(
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
                false)
        {
            SetText("%s", szContent);
        }

    public:
        ~LabelBoard() = default;

    public:
        bool Load(XMLObjectList &rstXMLObjectList)
        {
            return m_TokenBoard.Load(rstXMLObjectList);
        }

    public:
        const char *GetText() const
        {
            return m_Content.c_str();
        }

    public:
        void SetText(const char *, ...);

    public:
        void SetColor(const SDL_Color &rstColor)
        {
            if(false
                    || rstColor.r != m_FontColor.r
                    || rstColor.g != m_FontColor.g
                    || rstColor.b != m_FontColor.b
                    || rstColor.a != m_FontColor.a){

                m_FontColor = rstColor;

                auto szOld = m_Content;
                SetText(szOld.c_str());
            }
        }

    public:
        void Clear()
        {
            m_Content = "";
            m_TokenBoard.Clear();
        }

    public:
        std::string Print   () const;
        std::string PrintXML() const;

    public:
        void DrawEx(int nDstX, int nDstY, int nSrcX, int nSrcY, int nW, int nH)
        {
            m_TokenBoard.DrawEx(nDstX, nDstY, nSrcX, nSrcY, nW, nH);
        }
};
