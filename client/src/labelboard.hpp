/*
 * =====================================================================================
 *
 *       Filename: labelboard.hpp
 *        Created: 08/20/2015 08:59:11
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
#include <string>
#include <vector>
#include <cstdint>
#include <SDL2/SDL.h>

#include "widget.hpp"
#include "lalign.hpp"
#include "xmltypeset.hpp"
#include "colorf.hpp"

class LabelBoard: public Widget
{
    private:
        XMLTypeset m_tpset;

    public:
        LabelBoard(
                int            x,
                int            y,
                const char8_t *content    = u8"",
                uint8_t        font       = 0,
                uint8_t        fontSize   = 10,
                uint8_t        fontStyle  = 0,
                uint32_t       fontColor  = colorf::WHITE + 255,
                Widget        *widgetPtr  = nullptr,
                bool           autoDelete = false)
            : Widget(x, y, 0, 0, widgetPtr, autoDelete)
            , m_tpset
              {
                  0,
                  LALIGN_LEFT,
                  false,
                  font,
                  fontSize,
                  fontStyle,
                  fontColor,
              }
        {
            setText(u8"%s", content);
        }

    public:
        ~LabelBoard() = default;

    public:
        void loadXML(const char *);
        void setText(const char8_t *, ...);

    public:
        std::string getText(bool textOnly) const
        {
            return m_tpset.getText(textOnly);
        }

    public:
        void setFont(uint8_t nFont)
        {
            m_tpset.setFont(nFont);
        }

        void setFontSize(uint8_t nFontSize)
        {
            m_tpset.setFontSize(nFontSize);
        }

        void setFontStyle(uint8_t nFontStyle)
        {
            m_tpset.setFontStyle(nFontStyle);
        }

        void setFontColor(uint32_t nFontColor)
        {
            m_tpset.setFontColor(nFontColor);
        }

    public:
        void clear()
        {
            m_tpset.clear();
        }

    public:
        std::string PrintXML() const
        {
            return m_tpset.PrintXML();
        }

    public:
        void drawEx(int nDstX, int nDstY, int nSrcX, int nSrcY, int nW, int nH) override
        {
            m_tpset.drawEx(nDstX, nDstY, nSrcX, nSrcY, nW, nH);
        }

    public:
        void setImageMaskColor(uint32_t color)
        {
            m_tpset.setImageMaskColor(color);
        }
};
