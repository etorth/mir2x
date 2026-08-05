#pragma once
#include <functional>
#include "widget.hpp"
#include "textboard.hpp"
#include "textbutton.hpp"
#include "layoutboard.hpp"
#include "serdesmsg.hpp"

class AcutionItemDetailUpper final: public Widget
{
    public:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            std::function<void(const SDChatPeer &)> onContactSeller = nullptr;

            Widget::WADPair parent {};
        };

    private:
        std::string m_titleString;

    private:
        SDChatPeer m_seller;
        std::function<void(const SDChatPeer &)> m_onContactSeller;

    private:
        TextBoard m_title;

    private:
        Widget m_noteArea;
        LayoutBoard m_note;

    private:
        TextButton m_buttonContactSeller;

    public:
        explicit AcutionItemDetailUpper(AcutionItemDetailUpper::InitArgs);

    public:
        void setItem(const SDAcutionItem *);
};
