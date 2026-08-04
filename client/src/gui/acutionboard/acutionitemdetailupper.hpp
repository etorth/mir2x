#pragma once
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

            Widget::InstAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        TextBoard m_title;

    private:
        Widget m_noteArea;
        LayoutBoard m_note;

    private:
        Widget m_sellerArea;
        LayoutBoard m_seller;

    private:
        TextButton m_buttonContactSeller;

    public:
        explicit AcutionItemDetailUpper(AcutionItemDetailUpper::InitArgs);

    public:
        void setItem(const SDAcutionItem *);
};
