#pragma once
#include <functional>
#include "colorf.hpp"
#include "widget.hpp"
#include "xmltypeset.hpp"
#include "ime.hpp"

class InputLine: public Widget
{
    protected:
        XMLTypeset m_tpset;

    protected:
        int m_cursor = 0;

    protected:
        int      m_cursorWidth;
        uint32_t m_cursorColor;
        double   m_cursorBlink = 0.0;

    protected:
        std::function<void()> m_onTab;
        std::function<void()> m_onCR;

    protected:
        bool m_imeEnabled = true;

    public:
        InputLine(
                dir8_t   dir,
                int      x,
                int      y,
                int      w,
                int      h,

                uint8_t  font      =  0,
                uint8_t  fontSize  = 10,
                uint8_t  fontStyle =  0,
                uint32_t fontColor =  colorf::WHITE + colorf::A_SHF(255),

                int      cursorWidth = 2,
                uint32_t cursorColor = colorf::WHITE + colorf::A_SHF(255),

                std::function<void()>  onTab      = nullptr,
                std::function<void()>  onCR       = nullptr,
                Widget                *widgetPtr  = nullptr,
                bool                   autoDelete = false)
            : Widget(dir, x, y, w, h, widgetPtr, autoDelete)
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
            , m_onTab(onTab)
            , m_onCR(onCR)
        {}

    public:
        bool processEvent(const SDL_Event &, bool);

    public:
        void drawEx(int, int, int, int, int, int) const override;

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

        virtual void clear()
        {
            m_cursor = 0;
            m_cursorBlink = 0.0;
            m_tpset.clear();
        }

    public:
        void deleteChar();
        void insertChar(char);
};
