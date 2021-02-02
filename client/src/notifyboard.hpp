/*
 * =====================================================================================
 *
 *       Filename: notifyboard.hpp
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
#include <deque>
#include "widget.hpp"
#include "lalign.hpp"
#include "xmltypeset.hpp"

class NotifyBoard: public Widget
{
    private:
        int m_lineW;

    private:
        uint8_t m_font;
        uint8_t m_fontSize;
        uint8_t m_fontStyle;

    private:
        uint32_t m_fontColor;

    private:
        std::deque<std::shared_ptr<XMLTypeset>> m_boardList;

    public:
        NotifyBoard(
                int              nX,
                int              nY,
                int              nW,
                uint8_t          defaultFont      = 0,
                uint8_t          defaultFontSize  = 10,
                uint8_t          defaultFontStyle = 0,
                uint32_t         defaultFontColor = colorf::WHITE + 255,
                Widget          *widgetPtr        = nullptr,
                bool             autoDelete       = false)
            : Widget(nX, nY, 0, 0, widgetPtr, autoDelete)
            , m_lineW(nW)
            , m_font(defaultFont)
            , m_fontSize(defaultFontSize)
            , m_fontStyle(defaultFontStyle)
            , m_fontColor(defaultFontColor)
        {}

    public:
        void addLog(const char8_t *, ...);

    public:
        void SetFont(uint8_t nFont)
        {
            m_font = nFont;
        }

        void SetFontSize(uint8_t nFontSize)
        {
            m_fontSize = nFontSize;
        }

        void SetFontStyle(uint8_t nFontStyle)
        {
            m_fontStyle = nFontStyle;
        }

        void SetFontColor(uint32_t nFontColor)
        {
            m_fontColor = nFontColor;
        }

    public:
        void Clear()
        {
            m_boardList.clear();
            m_w = 0;
            m_h = 0;
        }

    public:
        int pw();

    public:
        void drawEx(int, int, int, int, int, int) const override;
};
