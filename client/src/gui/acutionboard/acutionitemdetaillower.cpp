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

    , m_imageArea
      {{
          .x = 7,
          .y = 8,
          .w = 56,
          .h = 56,
          .parent{this},
      }}

    , m_image
      {{
          .dir = DIR_NONE,
          .x = 28,
          .y = 28,
          .w = [this]{ return itemImageSize().first; },
          .h = [this]{ return itemImageSize().second; },
          .texLoadFunc = [this]{ return itemTexture(); },
          .parent{&m_imageArea},
      }}

    , m_summaryArea
      {{
          .x = 73,
          .y = 6,
          .w = 145,
          .h = 61,
          .parent{this},
      }}

    , m_summary
      {{
          .lineWidth = 145,
          .font
          {
              .id = 1,
              .size = 10,
          },
          .lineAlign = LALIGN_LEFT,
          .parent{&m_summaryArea},
      }}

    , m_descriptionTitle
      {{
          .dir = DIR_LEFT,
          .x = 7,
          .y = 71,
          .textFunc = to_cstr(u8"物品描述"),
          .font
          {
              .id = 1,
              .size = 10,
              .color = colorf::GREEN_A255,
          },
          .parent{this},
      }}

    , m_descriptionArea
      {{
          .x = 7,
          .y = 85,
          .w = 211,
          .h = 22,
          .parent{this},
      }}

    , m_description
      {{
          .lineWidth = 211,
          .font
          {
              .id = 1,
              .size = 10,
          },
          .lineAlign = LALIGN_JUSTIFY,
          .parent{&m_descriptionArea},
      }}

    , m_attributeTitle
      {{
          .dir = DIR_LEFT,
          .x = 7,
          .y = 111,
          .textFunc = to_cstr(u8"物品属性"),
          .font
          {
              .id = 1,
              .size = 10,
              .color = colorf::GREEN_A255,
          },
          .parent{this},
      }}

    , m_attributeArea
      {{
          .x = 7,
          .y = 125,
          .w = 211,
          .h = 36,
          .parent{this},
      }}

    , m_attribute
      {{
          .lineWidth = 211,
          .font
          {
              .id = 1,
              .size = 10,
          },
          .lineAlign = LALIGN_LEFT,
          .parent{&m_attributeArea},
      }}
{
    setShow(false);
}

void AcutionItemDetailLower::setItem(const SDAcutionItem *item)
{
    if(!item){
        m_itemID = 0;
        m_summary.clear();
        m_description.clear();
        m_attribute.clear();
        setShow(false);
        return;
    }

    const auto &ir = DBCOM_ITEMRECORD(item->item.itemID);
    fflassert(ir);
    m_itemID = item->item.itemID;

    const auto durationString = [&]() -> std::string
    {
        if(ir.equip.duration > 0){
            fflassert(item->item.duration[0] <= item->item.duration[1]);
            return str_printf("%zu / %zu", item->item.duration[0], item->item.duration[1]);
        }
        return "--";
    }();

    std::string summaryXML = "<layout>";
    summaryXML += xmlf::toParString("名称：%s", to_cstr(ir.name));
    summaryXML += xmlf::toParString("类型：%s", to_cstr(ir.type));
    summaryXML += str_printf(
            "<par>售价：<t color='%s'>%s</t></par>",
            item->price >= 10000000 ? "red" : item->price >= 1000000 ? "cyan" : "yellow",
            str_ksep(item->price).c_str());
    summaryXML += xmlf::toParString("持久：%s", durationString.c_str());
    summaryXML += "</layout>";
    m_summary.loadXML(summaryXML.c_str());

    const auto descriptionXML = str_printf(
            "<layout>%s</layout>",
            xmlf::toParString(
                "%s",
                str_haschar(ir.description) ? to_cstr(ir.description) : to_cstr(u8"游戏处于开发阶段，暂无物品描述。")).c_str());
    m_description.loadXML(descriptionXML.c_str());

    const auto attributeXML = item->item.getXMLLayout({}, SDItem::XMLLAYOUT_ATTRIBUTE);
    m_attribute.loadXML(to_cstr(attributeXML));

    setShow(true);
}

SDL_Texture *AcutionItemDetailLower::itemTexture() const
{
    if(!m_itemID){
        return nullptr;
    }

    const auto &ir = DBCOM_ITEMRECORD(m_itemID);
    fflassert(ir);

    if(auto texPtr = g_itemDB->retrieve(ir.pkgGfxID | 0X02000000)){
        return texPtr;
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
        return g_itemDB->retrieve(ir.pkgGfxID | 0X01000000);
    }
    return nullptr;
}

std::pair<int, int> AcutionItemDetailLower::itemImageSize() const
{
    constexpr int maxItemW = 56;
    constexpr int maxItemH = 56;

    if(auto texPtr = itemTexture()){
        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
        const auto ratio = std::max<double>({to_df(texW) / maxItemW, to_df(texH) / maxItemH, 1.0});
        return
        {
            to_d(std::lround(texW / ratio)),
            to_d(std::lround(texH / ratio)),
        };
    }
    return {0, 0};
}
