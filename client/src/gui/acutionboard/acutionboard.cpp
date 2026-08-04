#include <algorithm>
#include <cmath>
#include "strf.hpp"
#include "xmlf.hpp"
#include "colorf.hpp"
#include "client.hpp"
#include "dbcomid.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "layoutboard.hpp"
#include "textboard.hpp"
#include "processrun.hpp"
#include "acutionregisterboard.hpp"
#include "acutionboard.hpp"

extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern Client *g_client;

namespace
{
    SDL_Texture *getItemTexture(const ItemRecord &ir)
    {
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
}

AcutionBoard::AcutionBoard(ProcessRun *argProc, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .dir = DIR_NONE,
          .x = []{ return g_sdlDevice->getRendererWidth () / 2; },
          .y = []{ return g_sdlDevice->getRendererHeight() / 2; },
          .w = std::nullopt,
          .h = std::nullopt,
          .parent{argParent, argAutoDelete},
      }}

    , m_runProc(argProc)
    , m_background
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00001400); },
          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , m_itemList
      {{
          .x = 10,
          .y = 84,
          .parent{this},
      }}

    , m_buttonContactSeller
      {{
          .dir = DIR_UPRIGHT,
          .x = 704,
          .y = 141,
          .textFunc = to_cstr(u8"联系卖家"),
          .font
          {
              .id = 1,
              .size = 11,
          },
          .onTrigger = [](Widget *, int)
          {
          },
          .parent{this},
      }}

    , m_buttonPrevious
      {{
          .x = 21,
          .y = 360,
          .texIDList
          {
              .off  = 0X00000311,
              .on   = 0X00000310,
              .down = 0X00000312,
          },
          .onTrigger = [this](Widget *, int)
          {
              if(m_page > 0){
                  setPage(m_page - 1);
              }
          },
          .parent{this},
      }}

    , m_buttonNext
      {{
          .x = 72,
          .y = 360,
          .texIDList
          {
              .off  = 0X00000301,
              .on   = 0X00000300,
              .down = 0X00000302,
          },
          .onTrigger = [this](Widget *, int)
          {
              if(m_page + 1 < pageCount()){
                  setPage(m_page + 1);
              }
          },
          .parent{this},
      }}

    , m_buttonRefresh
      {{
          .x = 122,
          .y = 361,
          .texIDList
          {
              .off  = 0X00000331,
              .on   = 0X00000330,
              .down = 0X00000332,
          },
          .onTrigger = [this](Widget *, int)
          {
              const auto category = m_itemList.getItemList().category;
              if(category >= ACUTIONCAT_BEGIN && category < ACUTIONCAT_END){
                  g_client->send({CM_QUERYACUTIONITEMLIST, CMQueryAcutionItemList
                  {
                      .category = to_u8(category),
                  }});
              }
          },
          .parent{this},
      }}

    , m_buttonItemSearch
      {{
          .x = 412,
          .y = 360,
          .texIDList
          {
              .on   = 0X00001420,
              .down = 0X00001421,
          },
          .onTrigger = [](Widget *, int)
          {
          },
          .parent{this},
      }}

    , m_buttonSellerSearch
      {{
          .x = 412,
          .y = 390,
          .texIDList
          {
              .on   = 0X00001420,
              .down = 0X00001421,
          },
          .onTrigger = [](Widget *, int)
          {
          },
          .parent{this},
      }}

    , m_buttonRegister
      {{
          .x = 513,
          .y = 370,
          .texIDList
          {
              // looks there are two version of register button, in GameInter
              //
              // TMP001228.PNG vs TMP001904.PNG
              // they are identical expect 1 pixel offset
              //
              // zsdb uses 1228, but acution board frame image uses 1904
              // here use zsdb and off texture to redraw the button off state

              .off  = 0X000000B3,
              .on   = 0X000000B3,
              .down = 0X000000B4,
          },
          .onTrigger = [this](Widget *, int)
          {
              if(auto registerBoardPtr = dynamic_cast<AcutionRegisterBoard *>(m_runProc->getWidget("AcutionRegisterBoard"))){
                  registerBoardPtr->begin();
              }
          },
          .parent{this},
      }}

    , m_buttonBuy
      {{
          .x = 563,
          .y = 370,
          .texIDList
          {
              .on   = 0X000000B7,
              .down = 0X000000B8,
          },
          .onTrigger = [](Widget *, int)
          {
          },
          .parent{this},
      }}

    , m_buttonCancel
      {{
          .x = 643,
          .y = 370,
          .texIDList
          {
              .off  = 0X00000850, // same as buttonRegister
              .on   = 0X00000850,
              .down = 0X00000851,
          },
          .onTrigger = [](Widget *, int)
          {
          },
          .parent{this},
      }}

    , m_buttonClose
      {{
          .x = 680,
          .y = 396,
          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },
          .onTrigger = [this](Widget *, int)
          {
              setShow(false);
          },
          .parent{this},
      }}
{
    fflassert(m_runProc);
    m_buttonContactSeller.setShow([this]
    {
        return m_itemList.selectedIndex().has_value();
    });

    setShow(false);
    updatePageButtonState();
}

void AcutionBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    Widget::drawDefault(m);

    const auto drawText = [this, &m](std::string text, uint32_t color, dir8_t dir, int x, int y, int fontSize = 11)
    {
        TextBoard textBoard
        {{
            .textFunc = std::move(text),
            .font
            {
                .id = 1,
                .size = to_u8(fontSize),
                .color = color,
            },
        }};
        drawAsChild(&textBoard, dir, x, y, m);
    };

    const auto pageCountValue = pageCount();
    const auto categoryName = [this]() -> const char8_t *
    {
        switch(m_itemList.getItemList().category){
            case ACUTIONCAT_ALL     : return u8"所有物品";
            case ACUTIONCAT_DRESS   : return u8"衣服";
            case ACUTIONCAT_WEAPON  : return u8"武器";
            case ACUTIONCAT_NECKLACE: return u8"项链";
            case ACUTIONCAT_HELMET  : return u8"头盔";
            case ACUTIONCAT_RING    : return u8"戒指";
            case ACUTIONCAT_ARMRING : return u8"手镯";
            case ACUTIONCAT_SHOES   : return u8"鞋类";
            case ACUTIONCAT_POTION  : return u8"药品";
            case ACUTIONCAT_BOOK    : return u8"图书";
            case ACUTIONCAT_OTHER   : return u8"其他物品";
            default                 : return u8"寄售";
        }
    }();

    drawText(to_cstr(categoryName), colorf::YELLOW_A255, DIR_NONE, 61, 20, 12);
    drawText(to_cstr(str_printf(u8"共%zu件", m_itemList.getItemList().itemList.size())), colorf::YELLOW_A255, DIR_NONE, 240, 20, 12);
    drawText(pageCountValue ? to_cstr(str_printf(u8"第%zu/%zu页", m_page + 1, pageCountValue)) : to_cstr(u8"（空）"), colorf::YELLOW_A255, DIR_NONE, 419, 20, 12);

    drawText(to_cstr(u8"物品"), colorf::YELLOW_A255, DIR_NONE, 64, 62, 12);
    drawText(to_cstr(u8"寄售人"), colorf::YELLOW_A255, DIR_NONE, 188, 62, 12);
    drawText(to_cstr(u8"剩余"), colorf::YELLOW_A255, DIR_NONE, 293, 62, 12);
    drawText(to_cstr(u8"价格"), colorf::YELLOW_A255, DIR_NONE, 399, 62, 12);

    drawSelectedItem(m);
}

bool AcutionBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    const auto oldFirstIndex = m_itemList.firstIndex();
    if(m_itemList.processEventParent(event, valid, m)){
        if(oldFirstIndex != m_itemList.firstIndex()){
            m_page = m_itemList.firstIndex() / AcutionItemList::m_rowCount;
            updatePageButtonState();
        }
        return true;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_buttonContactSeller.show() && m_buttonContactSeller.processEventParent(event, valid, m)){ return true; }
    if(m_buttonPrevious    .processEventParent(event, valid, m)){ return true; }
    if(m_buttonNext        .processEventParent(event, valid, m)){ return true; }
    if(m_buttonRefresh     .processEventParent(event, valid, m)){ return true; }
    if(m_buttonItemSearch  .processEventParent(event, valid, m)){ return true; }
    if(m_buttonSellerSearch.processEventParent(event, valid, m)){ return true; }
    if(m_buttonRegister    .processEventParent(event, valid, m)){ return true; }
    if(m_buttonBuy         .processEventParent(event, valid, m)){ return true; }
    if(m_buttonCancel      .processEventParent(event, valid, m)){ return true; }
    if(m_buttonClose       .processEventParent(event, valid, m)){ return true; }

    switch(event.type){
        case SDL_EVENT_KEY_DOWN:
            {
                if(event.key.key == SDLK_ESCAPE){
                    setShow(false);
                    setFocus(false);
                    return true;
                }
                return consumeFocus(false);
            }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                return consumeFocus(m.in(to_d(event.button.x), to_d(event.button.y)));
            }
        case SDL_EVENT_MOUSE_MOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(to_d(event.motion.x), to_d(event.motion.y)) || focus())){
                    const int remapX = m.x - m.ro->x;
                    const int remapY = m.y - m.ro->y;
                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();

                    const int newX = std::clamp(remapX + to_d(event.motion.xrel), 0, rendererW - w());
                    const int newY = std::clamp(remapY + to_d(event.motion.yrel), 0, rendererH - h());
                    moveBy(newX - remapX, newY - remapY);
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

void AcutionBoard::setItemList(SDAcutionItemList sdAcutionItemList)
{
    m_itemList.setItemList(std::move(sdAcutionItemList));
    setPage(0);
}

size_t AcutionBoard::pageCount() const
{
    return (m_itemList.getItemList().itemList.size() + AcutionItemList::m_rowCount - 1) / AcutionItemList::m_rowCount;
}

void AcutionBoard::setPage(size_t page)
{
    if(const auto count = pageCount(); count > 0){
        m_page = std::min(page, count - 1);
        m_itemList.setFirstIndex(m_page * AcutionItemList::m_rowCount);
        m_itemList.setSelectedIndex(m_itemList.firstIndex());
    }
    else{
        m_page = 0;
        m_itemList.setFirstIndex(0);
        m_itemList.setSelectedIndex(0);
    }
    updatePageButtonState();
}

void AcutionBoard::updatePageButtonState()
{
    m_buttonPrevious.setActive(m_page > 0);
    m_buttonNext.setActive(m_page + 1 < pageCount());
    const auto category = m_itemList.getItemList().category;
    m_buttonRefresh.setActive(category >= ACUTIONCAT_BEGIN && category < ACUTIONCAT_END);
}

void AcutionBoard::drawSelectedItem(Widget::ROIMap m) const
{
    const auto selectedIndex = m_itemList.selectedIndex();
    if(!selectedIndex.has_value()){
        return;
    }

    const auto &entry = m_itemList.getItemList().itemList.at(selectedIndex.value());
    const auto &ir = DBCOM_ITEMRECORD(entry.item.itemID);
    fflassert(ir);

    const int remapX = m.x - m.ro->x;
    const int remapY = m.y - m.ro->y;

    const auto drawText = [this, &m](std::string text, uint32_t color, int x, int y, int fontSize = 11)
    {
        TextBoard textBoard
        {{
            .textFunc = std::move(text),
            .font
            {
                .id = 1,
                .size = to_u8(fontSize),
                .color = color,
            },
        }};
        drawAsChild(&textBoard, DIR_LEFT, x, y, m);
    };

    drawText(to_cstr(u8"卖家留言"), colorf::CYAN_A255, 493, 58, 11);

    const auto noteXML = str_printf(
            "<layout>%s</layout>",
            xmlf::toParString(
                "%s",
                entry.note.empty() ? to_cstr(u8"卖家未填写留言。") : entry.note.c_str()).c_str());

    LayoutBoard noteBoard
    {{
        .lineWidth = 211,
        .initXML = noteXML.c_str(),
        .font
        {
            .id = 1,
            .size = 10,
        },
        .lineAlign = LALIGN_JUSTIFY,
    }};
    noteBoard.draw({.x=remapX + 493, .y=remapY + 77, .ro{0, 0, 211, 52}});

    const auto sellerXML = str_printf(
            "<layout>%s</layout>",
            xmlf::toParString("卖家：%s", entry.seller.c_str()).c_str());
    LayoutBoard sellerBoard
    {{
        .lineWidth = 155,
        .initXML = sellerXML.c_str(),
        .font
        {
            .id = 1,
            .size = 10,
        },
        .lineAlign = LALIGN_LEFT,
    }};
    sellerBoard.draw({.x=remapX + 493, .y=remapY + 142, .ro{0, 0, 155, 15}});

    if(auto texPtr = getItemTexture(ir)){
        constexpr int maxItemW = 56;
        constexpr int maxItemH = 56;
        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
        const auto ratio = std::max<double>({to_df(texW) / maxItemW, to_df(texH) / maxItemH, 1.0});
        const int drawW = to_d(std::lround(texW / ratio));
        const int drawH = to_d(std::lround(texH / ratio));

        g_sdlDevice->drawTexture(
                texPtr,
                remapX + 493 + (maxItemW - drawW) / 2,
                remapY + 176 + (maxItemH - drawH) / 2,
                drawW,
                drawH,
                0,
                0,
                texW,
                texH);
    }

    const auto durationString = [&entry, &ir]() -> std::string
    {
        if(ir.equip.duration > 0){
            fflassert(entry.item.duration[0] <= entry.item.duration[1]);
            return str_printf("%zu / %zu", entry.item.duration[0], entry.item.duration[1]);
        }
        return "--";
    }();

    std::string summaryXML = "<layout>";
    summaryXML += xmlf::toParString("名称：%s", to_cstr(ir.name));
    summaryXML += xmlf::toParString("类型：%s", to_cstr(ir.type));
    summaryXML += str_printf(
            "<par>售价：<t color='%s'>%s</t></par>",
            entry.price >= 10000000 ? "red" : entry.price >= 1000000 ? "cyan" : "yellow",
            str_ksep(entry.price).c_str());
    summaryXML += xmlf::toParString("持久：%s", durationString.c_str());
    summaryXML += "</layout>";

    LayoutBoard summaryBoard
    {{
        .lineWidth = 145,
        .initXML = summaryXML.c_str(),
        .font
        {
            .id = 1,
            .size = 10,
        },
        .lineAlign = LALIGN_LEFT,
    }};
    summaryBoard.draw({.x=remapX + 559, .y=remapY + 174, .ro{0, 0, 145, 61}});

    drawText(to_cstr(u8"物品描述"), colorf::GREEN_A255, 493, 239, 10);
    const auto descriptionXML = str_printf(
            "<layout>%s</layout>",
            xmlf::toParString(
                "%s",
                str_haschar(ir.description) ? to_cstr(ir.description) : to_cstr(u8"游戏处于开发阶段，暂无物品描述。")).c_str());

    LayoutBoard descriptionBoard
    {
        {
            .lineWidth = 211,
            .initXML = descriptionXML.c_str(),
            .font
            {
                .id = 1,
                .size = 10,
            },
            .lineAlign = LALIGN_JUSTIFY,
        }
    };
    descriptionBoard.draw({.x=remapX + 493, .y=remapY + 253, .ro{0, 0, 211, 22}});

    drawText(to_cstr(u8"物品属性"), colorf::GREEN_A255, 493, 279, 10);
    const auto attributeXML = entry.item.getXMLLayout({}, SDItem::XMLLAYOUT_ATTRIBUTE);
    LayoutBoard attributeBoard
    {{
        .lineWidth = 211,
        .initXML = to_cstr(attributeXML),
        .font
        {
            .id = 1,
            .size = 10,
        },
        .lineAlign = LALIGN_LEFT,
    }};
    attributeBoard.draw({.x=remapX + 493, .y=remapY + 293, .ro{0, 0, 211, 36}});
}
