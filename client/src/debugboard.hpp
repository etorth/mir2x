/*
 * =====================================================================================
 *
 *       Filename: debugboard.hpp
 *        Created: 03/22/2020 16:43:00
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
#include "widget.hpp"
#include "lalign.hpp"
#include "xmltypeset.hpp"

class DebugBoard: public Widget
{
    private:
        int m_LineW;

    private:
        uint8_t m_DefaultFont;
        uint8_t m_DefaultFontSize;
        uint8_t m_DefaultFontStyle;

    private:
        uint32_t m_DefaultFontColor;

    private:
        std::deque<std::shared_ptr<XMLTypeset>> m_BoardList;

    public:
        DebugBoard(
                int              nX,
                int              nY,
                int              nW,
                uint8_t          nDefaultFont      = 0,
                uint8_t          nDefaultFontSize  = 10,
                uint8_t          nDefaultFontStyle = 0,
                uint32_t         nDefaultFontColor = ColorFunc::WHITE + 255,
                Widget          *pWidget           = nullptr,
                bool             bAutoDelete       = false)
            : Widget(nX, nY, 0, 0, pWidget, bAutoDelete)
            , m_LineW(nW)
            , m_DefaultFont(nDefaultFont)
            , m_DefaultFontSize(nDefaultFontSize)
            , m_DefaultFontStyle(nDefaultFontStyle)
            , m_DefaultFontColor(nDefaultFontColor)
        {}

    public:
        void addLog(const char *, ...);

    public:
        void SetFont(uint8_t nFont)
        {
            m_DefaultFont = nFont;
        }

        void SetFontSize(uint8_t nFontSize)
        {
            m_DefaultFontSize = nFontSize;
        }

        void SetFontStyle(uint8_t nFontStyle)
        {
            m_DefaultFontStyle = nFontStyle;
        }

        void SetFontColor(uint32_t nFontColor)
        {
            m_DefaultFontColor = nFontColor;
        }

    public:
        void Clear()
        {
            m_BoardList.clear();
            m_W = 0;
            m_H = 0;
        }

    public:
        int PW();

    public:
        void drawEx(int, int, int, int, int, int);
};
