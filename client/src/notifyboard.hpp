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
                dir8_t           nDir,
                int              nX,
                int              nY,
                int              nW,
                uint8_t          defaultFont      = 0,
                uint8_t          defaultFontSize  = 10,
                uint8_t          defaultFontStyle = 0,
                uint32_t         defaultFontColor = colorf::WHITE + colorf::A_SHF(255),
                uint64_t         showTime         = 0,
                size_t           maxEntryCount    = 0,
                Widget          *widgetPtr        = nullptr,
                bool             autoDelete       = false)
            : Widget(nDir, nX, nY, 0, 0, widgetPtr, autoDelete)
            , m_lineW(nW)
            , m_font(defaultFont)
            , m_fontSize(defaultFontSize)
            , m_fontStyle(defaultFontStyle)
            , m_fontColor(defaultFontColor)
            , m_showTime(showTime)
            , m_maxEntryCount(maxEntryCount)
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
        void clear()
        {
            m_boardList.clear();
            m_w = 0;
            m_h = 0;
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

    private:
        void updateSize();
};
