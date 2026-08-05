#pragma once
#include <cstdint>
#include <utility>
#include "widget.hpp"
#include "imageboard.hpp"
#include "textboard.hpp"
#include "layoutboard.hpp"
#include "margincontainer.hpp"
#include "itembox.hpp"
#include "scrollcontainer.hpp"
#include "serdesmsg.hpp"

class AcutionItemDetailLower final: public Widget
{
    private:
        constexpr static int m_maxItemImageW = 56;
        constexpr static int m_maxItemImageH = 56;

    private:
        constexpr static int m_viewportW = 218;
        constexpr static int m_viewportH = 168;
        constexpr static int m_scrollBarSize = 8;

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
        ImageBoard m_image;
        MarginContainer m_imageArea;

    private:
        Widget m_summaryArea;
        LayoutBoard m_summary;

    private:
        ItemBox m_hItemBox;

    private:
        Widget m_descriptionArea;
        TextBoard m_descriptionTitle;
        LayoutBoard m_description;

    private:
        Widget m_attributeArea;
        TextBoard m_attributeTitle;
        LayoutBoard m_attribute;

    private:
        ItemBox m_vItemBox;
        ScrollContainer m_scroll;

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
