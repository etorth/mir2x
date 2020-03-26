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
                // widget paramater
                int                x,
                int                y,
                int                w,
                bool               canSelect  =  false,

                // forward to XMLLayout
                std::array<int, 4> margin     =  {0, 0, 0, 0},
                bool               canThrough =  false,
                uint8_t            font       =  0,
                uint8_t            fontSize   = 10,
                uint8_t            fontStyle  =  0,
                uint32_t           fontColor  =  ColorFunc::WHITE + 255,
                int                lineAlign  =  LALIGN_LEFT,
                int                lineSpace  =  0,
                int                wordSpace  =  0,
                Widget            *parent     =  nullptr,
                bool               autoDelete =  false)
            : Widget(x, y, 0, 0, parent, autoDelete)
            , m_layout
              {
                  w,
                  margin,
                  canThrough,
                  font,
                  fontSize,
                  fontStyle,
                  fontColor,
                  lineAlign,
                  lineSpace,
                  wordSpace,
              }
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
        void addParXML(int, const std::array<int, 4> &, const char *);

    public:
        void Update(double ms) override
        {
            m_layout.update(ms);
        }

    public:
        int parCount() const
        {
            return m_layout.parCount();
        }
};
