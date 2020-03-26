/*
 * =====================================================================================
 *
 *       Filename: layoutboard.hpp
 *        Created: 03/25/2020 22:27:45
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
#include "xmllayout.hpp"

class layoutBoard: public Widget
{
    private:
        XMLLayout m_layout;

    private:
        bool m_canSelect;

    public:
        layoutBoard(
                int              x,
                int              y,
                int              w,
                bool             canSelect        = false,
                uint8_t          defaultFont      = 0,
                uint8_t          defaultFontSize  = 10,
                uint8_t          defaultFontStyle = 0,
                uint32_t         defaultFontColor = ColorFunc::WHITE + 255,
                Widget          *parent           = nullptr,
                bool             autoDelete       = false)
            : Widget(x, y, 0, 0, parent, autoDelete)
            , m_layout(w, defaultFont, defaultFontSize, defaultFontStyle, defaultFontColor)
            , m_canSelect(canSelect)
        {}

    public:
        void drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) override
        {
            m_layout.drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
        }

    public:
        void loadXML(const char *);

    public:
        void Update(double ms) override
        {
            m_layout.update(ms);
        }
};
