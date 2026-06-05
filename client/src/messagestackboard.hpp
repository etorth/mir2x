#pragma once
#include <deque>
#include <memory>
#include <string>
#include "itemflex.hpp"
#include "lalign.hpp"
#include "marginwrapper.hpp"
#include "raiitimer.hpp"
#include "xmltypeset.hpp"

class MessageStackBoard: public Widget
{
    public:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            int width = 0;
            int corner = 0;

            Widget::FontConfig font {};

            uint64_t showTime = 0;
            size_t entryLimit = 0;

            ItemAlign align = ItemAlign::UPLEFT;
            Widget::VarSize itemSpace = 0;
            Widget::VarMargin margin {};

            Widget::VarU32 bgColor = colorf::RGBA(0X00, 0X00, 0X00, 0X80);
            Widget::VarU32 borderColor = colorf::RGBA(0XFF, 0XFF, 0XFF, 0X60);

            Widget::WADPair parent {};
        };

    private:
        struct Message final
        {
            hres_timer timer;
            std::unique_ptr<XMLTypeset> typeset;
            std::unique_ptr<MarginWrapper> wrapper;
        };

    private:
        ItemFlex m_itemFlex;

    private:
        int m_width;
        int m_corner;

    private:
        Widget::FontConfig m_font;

    private:
        uint64_t m_showTime;
        size_t m_entryLimit;

    private:
        Widget::VarMargin m_margin;

    private:
        Widget::VarU32 m_bgColor;
        Widget::VarU32 m_borderColor;

    private:
        std::deque<std::unique_ptr<Message>> m_messageList;

    public:
        explicit MessageStackBoard(MessageStackBoard::InitArgs);

    public:
        void addMessage(const std::u8string &);
        void addXMLMessage(const std::u8string &);
        void clear();

    public:
        bool empty() const;

    public:
        void setFont(uint8_t);
        void setFontSize(uint8_t);
        void setFontStyle(uint8_t);
        void setFontColor(Widget::VarU32);

    protected:
        void updateDefault(double) override;

    private:
        void removeFrontMessage();
};
