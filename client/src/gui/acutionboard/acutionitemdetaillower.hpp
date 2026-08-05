#pragma once
#include <cstdint>
#include <utility>
#include "widget.hpp"
#include "imageboard.hpp"
#include "textboard.hpp"
#include "layoutboard.hpp"
#include "serdesmsg.hpp"

class AcutionItemDetailLower final: public Widget
{
    private:
        constexpr static int m_maxItemImageW = 56;
        constexpr static int m_maxItemImageH = 56;

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
        uint32_t m_itemID = 0;

    private:
        Widget m_imageArea;
        ImageBoard m_image;

    private:
        Widget m_summaryArea;
        LayoutBoard m_summary;

    private:
        TextBoard m_descriptionTitle;
        Widget m_descriptionArea;
        LayoutBoard m_description;

    private:
        TextBoard m_attributeTitle;
        Widget m_attributeArea;
        LayoutBoard m_attribute;

    public:
        explicit AcutionItemDetailLower(AcutionItemDetailLower::InitArgs);

    public:
        void setItem(const SDAcutionItem *);

    private:
        SDL_Texture *itemTexture() const;
        std::pair<int, int> itemImageSize() const;

    private:
        static const char *priceColor(size_t);
};
