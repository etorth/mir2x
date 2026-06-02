#pragma once
#include <deque>
#include <memory>
#include <string>
#include "widget.hpp"
#include "lalign.hpp"
#include "raiitimer.hpp"
#include "xmltypeset.hpp"

class PlayerSayBoard: public Widget
{
    public:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            int lineW = 160;

            uint8_t font = 0;
            uint8_t fontSize = 12;
            uint8_t fontStyle = 0;

            Widget::VarU32 fontColor = colorf::RGBA(0XFF, 0XFF, 0XE0, 0XFF);

            uint64_t showTime = 5000;
            size_t maxEntryCount = 3;

            Widget::WADPair parent {};
        };

    private:
        struct Line final
        {
            hres_timer timer;
            std::unique_ptr<XMLTypeset> typeset;
        };

    private:
        static constexpr int m_paddingX = 4;
        static constexpr int m_paddingY = 2;

    private:
        int m_lineW = 0;
        uint8_t m_font = 0;
        uint8_t m_fontSize = 10;
        uint8_t m_fontStyle = 0;
        Widget::VarU32 m_fontColor = colorf::WHITE_A255;
        uint64_t m_showTime = 0;
        size_t m_maxEntryCount = 0;
        std::deque<Line> m_lineList;

    public:
        explicit PlayerSayBoard(PlayerSayBoard::InitArgs);

    public:
        void addSay(const std::string &);
        bool empty() const;
        int pw() const;

    public:
        void updateDefault(double) override;
        void drawDefault(Widget::ROIMap) const override;
};
