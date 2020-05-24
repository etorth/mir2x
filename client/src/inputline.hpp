/*
 * =====================================================================================
 *
 *       Filename: inputline.hpp
 *        Created: 06/19/2017 11:05:07
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
#include <functional>
#include "colorf.hpp"
#include "inputboard.hpp"
#include "xmltypeset.hpp"

class InputLine: public Widget
{
    protected:
        XMLTypeset m_tpset;

    protected:
        int m_cursor = 0;

    protected:
        int m_tpsetX = 0;
        int m_tpsetY = 0;

    protected:
        int      m_cursorWidth;
        uint32_t m_cursorColor;
        double   m_cursorBlink = 0.0;

    protected:
        std::function<void()> m_tabCB;
        std::function<void()> m_returnCB;

    public:
        InputLine(
                int      x,
                int      y,
                int      w,
                int      h,

                uint8_t  font      =  0,
                uint8_t  fontSize  = 10,
                uint8_t  fontStyle =  0,
                uint32_t fontColor =  colorf::WHITE,

                int      cursorWidth = 2,
                uint32_t cursorColor = colorf::WHITE,

                std::function<void()>  fnOnTab    = [](){},
                std::function<void()>  fnOnReturn = [](){},
                Widget                *parent     = nullptr,
                bool                   autoDelete = false)
            : Widget(x, y, w, h, parent, autoDelete)
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
            , m_cursorWidth(cursorWidth)
            , m_cursorColor(cursorColor)
            , m_tabCB(fnOnTab)
            , m_returnCB(fnOnReturn)
        {}

    public:
        bool processEvent(const SDL_Event &, bool);

    public:
        void drawEx(int, int, int, int, int, int) override;

    public:
        void update(double ms) override
        {
            m_cursorBlink += ms;
        }

    public:
        std::string getRawString() const
        {
            return m_tpset.getRawString();
        }

        void clear()
        {
            m_cursor = 0;
            m_cursorBlink = 0.0;
            m_tpset.clear();
        }

    public:
        void deleteChar();
        void insertChar(char);
};
