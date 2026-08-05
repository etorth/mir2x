#include <algorithm>
#include <cmath>
#include <utility>
#include "strf.hpp"
#include "xmlf.hpp"
#include "colorf.hpp"
#include "dbcomid.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "acutionitemdetaillower.hpp"

extern PNGTexDB *g_itemDB;
extern SDLDevice *g_sdlDevice;

AcutionItemDetailLower::AcutionItemDetailLower(AcutionItemDetailLower::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),
          .x = std::move(args.x),
          .y = std::move(args.y),
          .w = 226,
          .h = 168,
          .attrs
          {
              .inst = std::move(args.attrs),
          },
          .parent = std::move(args.parent),
      }}

    , m_image
      {{
          .w = [this]{ return itemImageSize().first; },
          .h = [this]{ return itemImageSize().second; },
          .texLoadFunc = [this]{ return itemTexture(); },
      }}

    , m_imageArea
      {{
          .w = m_maxItemImageW + 2,
          .h = m_maxItemImageH + 2,

          .contained
          {
              .dir = DIR_NONE,
              .widget = &m_image,
              .autoDelete = false,
          },

          .fgDrawFunc = [](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 100), drawDstX, drawDstY, self->w(), self->h());
          },
      }}

    , m_summaryArea
      {{
          .w = 145,
          .h = [this]{ return std::max<int>(61, m_summary.h()); },
      }}

    , m_summary
      {{
          .lineWidth = 145,
          .lineAlign = LALIGN_LEFT,
          .parent{&m_summaryArea},
      }}

    , m_hItemBox
      {{
          .v = false,
          .align = ItemAlign::CENTER,

          .headSpace = 7,
          .itemSpace = 8,

          .childList
          {
              {&m_imageArea, false},
              {&m_summaryArea, false},
          },
      }}

    , m_descriptionArea
      {{
          .w = m_viewportW,
          .h = [this]{ return m_descriptionTitle.h() + std::max<int>(22, m_description.h()); },
      }}

    , m_descriptionTitle
      {{
          .dir = DIR_LEFT,
          .x = 7,
          .textFunc = to_cstr(u8"物品描述"),
          .font
          {
              .color = colorf::GREEN_A255,
          },
          .parent{&m_descriptionArea},
      }}

    , m_description
      {{
          .x = 7,
          .y = [this]{ return m_descriptionTitle.h(); },
          .lineWidth = 211,
          .lineAlign = LALIGN_JUSTIFY,
          .parent{&m_descriptionArea},
      }}

    , m_attributeArea
      {{
          .w = m_viewportW,
          .h = [this]{ return m_attributeTitle.h() + std::max<int>(36, m_attribute.h()); },
      }}

    , m_attributeTitle
      {{
          .dir = DIR_LEFT,
          .x = 7,
          .textFunc = to_cstr(u8"物品属性"),
          .font
          {
              .color = colorf::GREEN_A255,
          },
          .parent{&m_attributeArea},
      }}

    , m_attribute
      {{
          .x = 7,
          .y = [this]{ return m_attributeTitle.h(); },
          .lineWidth = 211,
          .lineAlign = LALIGN_LEFT,
          .parent{&m_attributeArea},
      }}

    , m_vItemBox
      {{
          .fixed = m_viewportW,
          .v = true,
          .align = ItemAlign::UPLEFT,

          .headSpace = 6,
          .itemSpace = 4,
          .tailSpace = 7,

          .childList
          {
              {&m_hItemBox, false},
              {&m_descriptionArea, false},
              {&m_attributeArea, false},
          },
      }}

    , m_scroll
      {{
          .vpw = m_viewportW,
          .vph = m_viewportH,

          .getter = &m_vItemBox,

          .hBar = false,
          .vBar = true,

          .hScroll = false,
          .vScroll = true,

          .barSize = m_scrollBarSize,
          .parent{this},
      }}
{
    setShow(false);
}

void AcutionItemDetailLower::setItem(const SDAcutionItem *item)
{
    if(item){
        const auto &ir = DBCOM_ITEMRECORD(item->item.itemID);
        fflassert(ir);
        m_itemID = item->item.itemID;

        std::string summaryXML = "<layout>";
        summaryXML += xmlf::toParString("名称：%s", to_cstr(ir.name));
        summaryXML += xmlf::toParString("类型：%s", to_cstr(ir.type));
        summaryXML += str_printf("<par>售价：<t color='%s'>%s</t></par>", priceColor(item->price), to_cstr(str_ksep(item->price)));

        if(ir.equip.duration > 0){
            fflassert(item->item.duration[0] <= item->item.duration[1]);
            summaryXML += xmlf::toParString("持久：%zu / %zu", item->item.duration[0], item->item.duration[1]);
        }

        summaryXML += "</layout>";
        m_summary.loadXML(summaryXML.c_str());

        const auto descriptionXML = str_printf("<layout>%s</layout>", xmlf::toParString("%s", str_haschar(ir.description) ? to_cstr(ir.description) : to_cstr(u8"游戏处于开发阶段，暂无物品描述。")).c_str());
        m_description.loadXML(descriptionXML.c_str());

        const auto attributeXML = item->item.getXMLLayout({}, SDItem::XMLLAYOUT_ATTRIBUTE);
        m_attribute.loadXML(to_cstr(attributeXML));

        m_hItemBox.buildLayout();
        m_vItemBox.buildLayout();
        m_scroll.scrollTo(0, 0);

        setShow(true);
    }
    else{
        m_itemID = 0;

        m_summary.clear();
        m_description.clear();
        m_attribute.clear();

        m_hItemBox.buildLayout();
        m_vItemBox.buildLayout();
        m_scroll.scrollTo(0, 0);

        setShow(false);
    }
}

SDL_Texture *AcutionItemDetailLower::itemTexture() const
{
    if(!m_itemID){
        return nullptr;
    }

    const auto &ir = DBCOM_ITEMRECORD(m_itemID);
    fflassert(ir);

    if(auto texPtr = g_itemDB->retrieve(ir.pkgGfxID | 0X02000000)){
        return texPtr; // items in shop grid, smaller
    }

    if(false
            || ir.wearable(WLG_DRESS)
            || ir.wearable(WLG_HELMET)
            || ir.wearable(WLG_WEAPON)
            || ir.wearable(WLG_SHOES)
            || ir.wearable(WLG_NECKLACE)
            || ir.wearable(WLG_ARMRING0)
            || ir.wearable(WLG_RING0)
            || ir.wearable(WLG_TORCH)){
        return g_itemDB->retrieve(ir.pkgGfxID | 0X01000000); // items in invpack, can be bigger
    }
    return nullptr;
}

std::pair<int, int> AcutionItemDetailLower::itemImageSize() const
{
    if(auto texPtr = itemTexture()){
        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
        const auto bestRatio = std::max<double>
        ({
                1.0, // don't room in when image is small
                to_df(texW) / m_maxItemImageW,
                to_df(texH) / m_maxItemImageH,
        });

        return
        {
            to_d(std::lround(texW / bestRatio)),
            to_d(std::lround(texH / bestRatio)),
        };
    }
    return {0, 0};
}

const char *AcutionItemDetailLower::priceColor(size_t price)
{
    if(price >= 10000000){
        return "red";
    }
    else if(price >= 1000000){
        return "cyan";
    }
    else if(price >= 100000){
        return "yellow";
    }
    else{
        return "white";
    }
}
