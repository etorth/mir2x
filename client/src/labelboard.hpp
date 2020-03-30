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
#include "colorfunc.hpp"

class labelBoard: public widget
{
    private:
        XMLTypeset m_tpset;

    public:
        labelBoard(
                int         x,
                int         y,
                const char *content     =  "",
                uint8_t     font        =  0,
                uint8_t     fontSize    = 10,
                uint8_t     fontStyle   =  0,
                uint32_t    fontColor   =  ColorFunc::WHITE + 255,
                widget     *pwidget     =  nullptr,
                bool        bAutoDelete =  false)
            : widget(x, y, 0, 0, pwidget, bAutoDelete)
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
        ~labelBoard() = default;

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
        void SetFont(uint8_t nFont)
        {
            m_tpset.SetDefaultFont(nFont);
        }

        void SetFontSize(uint8_t nFontSize)
        {
            m_tpset.SetDefaultFontSize(nFontSize);
        }

        void SetFontStyle(uint8_t nFontStyle)
        {
            m_tpset.SetDefaultFontStyle(nFontStyle);
        }

        void SetFontColor(uint32_t nFontColor)
        {
            m_tpset.SetDefaultFontColor(nFontColor);
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
