#pragma once
#include <deque>
#include <memory>
#include "widget.hpp"
#include "lalign.hpp"
#include "raiitimer.hpp"
#include "xmltypeset.hpp"

class NotifyBoard: public Widget
{
    struct NotifyBoardTypeset
    {
        hres_timer timer;
        std::unique_ptr<XMLTypeset> typeset;
    };

    private:
        int m_lineW;

    private:
        uint8_t m_font;
        uint8_t m_fontSize;
        uint8_t m_fontStyle;

    private:
        uint32_t m_fontColor;

    private:
        uint64_t m_showTime;
        size_t m_maxEntryCount;

    private:
        std::deque<NotifyBoardTypeset> m_boardList;

    public:
        NotifyBoard(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                int, // line width

                uint8_t =  0,
                uint8_t = 10,
                uint8_t =  0,

                uint32_t = colorf::WHITE + colorf::A_SHF(255),

                uint64_t = 0,
                size_t   = 0,

                Widget * = nullptr,
                bool     = false);

    public:
        void addLog(const char8_t *, ...);

    public:
        void setFont(uint8_t argFont)
        {
            m_font = argFont;
        }

        void setFontSize(uint8_t argFontSize)
        {
            m_fontSize = argFontSize;
        }

        void setFontStyle(uint8_t argFontStyle)
        {
            m_fontStyle = argFontStyle;
        }

        void setFontColor(uint32_t argFontColor)
        {
            m_fontColor = argFontColor;
        }

    public:
        void clear()
        {
            m_boardList.clear();
        }

    public:
        int pw() const;

    public:
        bool empty() const
        {
            return m_boardList.empty();
        }

    public:
        void update(double) override;

    public:
        void drawEx(int, int, int, int, int, int) const override;
};
