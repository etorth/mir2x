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
                int         x,
                int         y,
                const char *content     =  "",
                uint8_t     font        =  0,
                uint8_t     fontSize    = 10,
                uint8_t     fontStyle   =  0,
                uint32_t    fontColor   =  colorf::WHITE + 255,
                Widget     *pwidget     =  nullptr,
                bool        bAutoDelete =  false)
            : Widget(x, y, 0, 0, pwidget, bAutoDelete)
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
            setText("%s", content);
        }

    public:
        ~LabelBoard() = default;

    public:
        void loadXML(const char *szXMLString)
        {
            m_tpset.loadXML(szXMLString);
        }

    public:
        void setText(const char *, ...);

    public:
        std::string GetText(bool bTextOnly) const
        {
            return m_tpset.GetText(bTextOnly);
        }

    public:
        void setFont(uint8_t nFont)
        {
            m_tpset.setDefaultFont(nFont);
        }

        void setFontSize(uint8_t nFontSize)
        {
            m_tpset.setDefaultFontSize(nFontSize);
        }

        void setFontStyle(uint8_t nFontStyle)
        {
            m_tpset.setDefaultFontStyle(nFontStyle);
        }

        void setFontColor(uint32_t nFontColor)
        {
            m_tpset.setDefaultFontColor(nFontColor);
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
        void drawEx(int nDstX, int nDstY, int nSrcX, int nSrcY, int nW, int nH)
        {
            m_tpset.drawEx(nDstX, nDstY, nSrcX, nSrcY, nW, nH);
        }
};
