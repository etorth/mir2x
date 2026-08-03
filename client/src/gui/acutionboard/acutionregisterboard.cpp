#include <algorithm>
#include <charconv>
#include <cmath>
#include <limits>
#include "strf.hpp"
#include "utf8f.hpp"
#include "mathf.hpp"
#include "xmlf.hpp"
#include "colorf.hpp"
#include "client.hpp"
#include "dbcomid.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "textboard.hpp"
#include "processrun.hpp"
#include "inventoryboard.hpp"
#include "inputstringboard.hpp"
#include "acutionregisterboard.hpp"

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

AcutionRegisterBoard::AcutionRegisterBoard(ProcessRun *argProc, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .dir = DIR_NONE,
          .x = [](const Widget *){ return g_sdlDevice->getRendererWidth () / 2; },
          .y = [](const Widget *){ return g_sdlDevice->getRendererHeight() / 2; },
          .w = std::nullopt,
          .h = std::nullopt,
          .parent{argParent, argAutoDelete},
      }}

    , m_runProc(argProc)
    , m_background
      {{
          .texLoadFunc = [](const Widget *) -> SDL_Texture *
          {
              return g_progUseDB->retrieve(0X00001410);
          },
          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , m_noteArea
      {{
          .x = m_noteX,
          .y = m_noteY,
          .w = m_noteW,
          .h = m_noteH,
          .parent{this},
      }}

    , m_noteBoard
      {{
          .lineWidth = m_noteW,
          .canEdit = true,
          .enableIME = [this]
          {
              return m_runProc->getRuntimeConfig<RTCFG_IME>();
          },
          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::WHITE_A255,
          },
          .lineAlign = LALIGN_LEFT,
          .parent{&m_noteArea},
      }}

    , m_buttonRegister
      {{
          .x = 248,
          .y = 229,
          .texIDList
          {
              .on   = 0X000000B3,
              .down = 0X000000B4,
          },
          .onTrigger = [this](Widget *, int)
          {
              confirmRegister();
          },
          .parent{this},
      }}

    , m_buttonCancel
      {{
          .x = 318,
          .y = 229,
          .texIDList
          {
              .on   = 0X00000850,
              .down = 0X00000851,
          },
          .onTrigger = [this](Widget *, int)
          {
              confirmCancel();
          },
          .parent{this},
      }}

    , m_buttonClose
      {{
          .x = 368,
          .y = 241,
          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },
          .onTrigger = [this](Widget *, int)
          {
              confirmCancel();
          },
          .parent{this},
      }}
{
    fflassert(m_runProc);
    setShow(false);
}

void AcutionRegisterBoard::begin()
{
    auto invBoardPtr = dynamic_cast<InventoryBoard *>(m_runProc->getWidget("InventoryBoard"));
    fflassert(invBoardPtr);

    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
    const int totalW = invBoardPtr->w() + w();
    const int startX = std::max(0, (rendererW - totalW) / 2);

    invBoardPtr->moveAt(
            DIR_UPLEFT,
            startX,
            std::max(0, (rendererH - invBoardPtr->h()) / 2));

    moveAt(
            DIR_UPLEFT,
            startX + invBoardPtr->w(),
            std::max(0, (rendererH - h()) / 2));

    invBoardPtr->setActive(!m_pending);
    invBoardPtr->setShow(true);
    setShow(true);
    setFocus(true);
}

void AcutionRegisterBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    Widget::drawDefault(m);

    const int remapX = m.x - m.ro->x;
    const int remapY = m.y - m.ro->y;

    if(!m_noteBoard.hasToken() && !m_noteBoard.focus()){
        const TextBoard placeholder
        {{
            .textFunc = to_cstr(u8"可填写联系方式或其它说明"),
            .font
            {
                .id = 1,
                .size = 11,
                .color = colorf::RGBA(220, 220, 220, 128),
            },
        }};
        drawAsChild(&placeholder, DIR_UPLEFT, m_noteX + 2, m_noteY + 2, m);
    }

    const auto drawText = [this, &m](std::string text, uint32_t color, dir8_t dir, int x, int y, int fontSize = 11)
    {
        const TextBoard textBoard
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

    if(m_item){
        const auto &ir = DBCOM_ITEMRECORD(m_item.itemID);
        fflassert(ir);

        if(auto texPtr = getItemTexture(ir)){
            constexpr int maxItemW = m_itemW - 20;
            constexpr int maxItemH = m_itemH - 40;
            const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
            const auto ratio = std::max<double>({to_df(texW) / maxItemW, to_df(texH) / maxItemH, 1.0});
            const int drawW = to_d(std::lround(texW / ratio));
            const int drawH = to_d(std::lround(texH / ratio));

            g_sdlDevice->drawTexture(
                    texPtr,
                    remapX + m_itemX + (m_itemW - drawW) / 2,
                    remapY + m_itemY + 5 + (maxItemH - drawH) / 2,
                    drawW,
                    drawH,
                    0,
                    0,
                    texW,
                    texH);
        }
        drawText(to_cstr(ir.name), colorf::YELLOW_A255, DIR_NONE, m_itemX + m_itemW / 2, m_itemY + m_itemH - 18, 12);
    }

    drawText(m_price ? str_ksep(m_price) : to_cstr(u8"点击设置价格"), m_price ? colorf::YELLOW_A255 : colorf::WHITE_A255, DIR_NONE, m_priceX + m_priceW / 2, m_priceY + m_priceH / 2, 12);
}

bool AcutionRegisterBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE){
        if(!m_pending){
            confirmCancel();
        }
        return true;
    }

    if(m_noteArea      .processEventParent(event, valid && !m_pending, m)){ return true; }
    if(m_buttonRegister.processEventParent(event, valid, m)){ return true; }
    if(m_buttonCancel  .processEventParent(event, valid, m)){ return true; }
    if(m_buttonClose   .processEventParent(event, valid, m)){ return true; }

    switch(event.type){
        case SDL_EVENT_KEY_DOWN:
            {
                return consumeFocus(false);
            }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                m_dragging = false;
                if(event.button.button == SDL_BUTTON_LEFT){
                    const int remapX = m.x - m.ro->x;
                    const int remapY = m.y - m.ro->y;
                    const int localX = to_d(event.button.x) - remapX;
                    const int localY = to_d(event.button.y) - remapY;

                    if(mathf::pointInRectangle<int>(localX, localY, m_noteX, m_noteY, m_noteW, m_noteH)){
                        if(!m_pending){
                            m_noteBoard.setFocus(true);
                        }
                        return consumeFocus(true);
                    }

                    if(mathf::pointInRectangle<int>(localX, localY, m_itemX, m_itemY, m_itemW, m_itemH)){
                        if(!m_pending){
                            auto &invPack = m_runProc->getMyHero()->getInvPack();
                            if(const auto grabbedItem = invPack.getGrabbedItem()){
                                restoreOwnedItem();
                                m_item = grabbedItem;
                                invPack.setGrabbedItem({});
                                m_price = 0;
                                m_noteBoard.setFocus(true);
                            }
                            else if(m_item){
                                invPack.setGrabbedItem(m_item);
                                m_item = {};
                                m_price = 0;
                            }
                        }
                        return consumeFocus(true);
                    }

                    if(mathf::pointInRectangle<int>(localX, localY, m_priceX, m_priceY, m_priceW, m_priceH)){
                        if(!m_pending && m_item){
                            setPrice();
                        }
                        return consumeFocus(true);
                    }

                    if(m_pending){
                        const auto inButton = [localX, localY](const Widget &button)
                        {
                            return mathf::pointInRectangle<int>(localX, localY, button.dx(), button.dy(), button.w(), button.h());
                        };

                        if(inButton(m_buttonRegister) || inButton(m_buttonCancel) || inButton(m_buttonClose)){
                            return consumeFocus(true);
                        }
                    }

                    m_dragging = m.in(to_d(event.button.x), to_d(event.button.y));
                }
                return consumeFocus(m.in(to_d(event.button.x), to_d(event.button.y)));
            }
        case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                m_dragging = false;
                return consumeFocus(m.in(to_d(event.button.x), to_d(event.button.y)));
            }
        case SDL_EVENT_MOUSE_MOTION:
            {
                if(m_dragging && (event.motion.state & SDL_BUTTON_LMASK)){
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

void AcutionRegisterBoard::setPending(bool pending)
{
    m_pending = pending;
    m_noteArea      .setActive(!pending);
    m_buttonRegister.setActive(!pending);
    m_buttonCancel  .setActive(!pending);
    m_buttonClose   .setActive(!pending);

    if(auto invBoardPtr = dynamic_cast<InventoryBoard *>(m_runProc->getWidget("InventoryBoard"))){
        invBoardPtr->setActive(!pending);
    }
}

void AcutionRegisterBoard::setPrice()
{
    auto inputBoardPtr = dynamic_cast<InputStringBoard *>(m_runProc->getWidget("InputStringBoard"));
    fflassert(inputBoardPtr);

    inputBoardPtr->waitInput(u8"<layout><par>你想卖多少钱呢?</par></layout>", false, [this](std::u8string input)
    {
        const std::string value(reinterpret_cast<const char *>(input.data()), input.size());
        uint64_t price = 0;
        const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), price, 10);
        if(value.empty()
                || ec != std::errc()
                || ptr != value.data() + value.size()
                || price == 0
                || price > to_u64(std::numeric_limits<int64_t>::max())){
            m_runProc->addCBLog(CBLOG_ERR, u8"无效的寄售价格：%s", value.c_str());
            return;
        }
        m_price = price;
    });
}

void AcutionRegisterBoard::confirmRegister()
{
    if(m_pending){
        return;
    }

    if(!m_item){
        m_runProc->addCBLog(CBLOG_ERR, u8"请先放入寄售物品");
        return;
    }

    if(m_price == 0 || m_price > to_u64(std::numeric_limits<int64_t>::max())){
        m_runProc->addCBLog(CBLOG_ERR, u8"请先设置有效的寄售价格");
        return;
    }

    const std::string note = m_noteBoard.getText();
    if(!utf8f::valid(note)){
        m_runProc->addCBLog(CBLOG_ERR, u8"寄售留言不是有效的UTF-8文本");
        return;
    }

    if(note.size() > SYS_ACUTIONNOTESIZE){
        m_runProc->addCBLog(CBLOG_ERR, u8"寄售留言不能超过%zu字节", SYS_ACUTIONNOTESIZE);
        return;
    }

    const auto &ir = DBCOM_ITEMRECORD(m_item.itemID);
    fflassert(ir);

    auto inputBoardPtr = dynamic_cast<InputStringBoard *>(m_runProc->getWidget("InputStringBoard"));
    fflassert(inputBoardPtr);

    std::u8string prompt = to_u8rawstr("<layout>"
            + xmlf::toParString("你确定上架%s吗?", to_cstr(ir.name))
            + xmlf::toParString("委托交易将会扣除手续费500，交易成功后扣除2%%交易额")
            + "</layout>");

    inputBoardPtr->waitInput(std::move(prompt), false, [this](std::u8string)
    {
        registerItem();
    });
}

void AcutionRegisterBoard::registerItem()
{
    if(m_pending || !m_item){
        return;
    }

    const std::string note = m_noteBoard.getText();
    if(!utf8f::valid(note)){
        m_runProc->addCBLog(CBLOG_ERR, u8"寄售留言不是有效的UTF-8文本");
        return;
    }

    if(note.size() > SYS_ACUTIONNOTESIZE){
        m_runProc->addCBLog(CBLOG_ERR, u8"寄售留言不能超过%zu字节", SYS_ACUTIONNOTESIZE);
        return;
    }

    if(m_price == 0 || m_price > to_u64(std::numeric_limits<int64_t>::max())){
        m_runProc->addCBLog(CBLOG_ERR, u8"寄售价格无效");
        return;
    }

    const auto &ir = DBCOM_ITEMRECORD(m_item.itemID);
    fflassert(ir);
    const std::string itemName = to_cstr(ir.name);

    CMRegisterAcutionItem cmRAI
    {
        .itemID = m_item.itemID,
        .seqID = m_item.seqID,
        .price = m_price,
    };
    cmRAI.note.assign(note);

    setPending(true);
    g_client->send({CM_REGISTERACUTIONITEM, cmRAI}, [itemName, this](uint8_t headCode, const uint8_t *buf, size_t bufSize)
    {
        switch(headCode){
            case SM_OK:
                {
                    m_runProc->addCBLog(CBLOG_SYS, u8"%s寄售成功", itemName.c_str());
                    closeRegister(true);
                    return;
                }
            case SM_ERROR:
                {
                    setPending(false);
                    if(buf && bufSize >= sizeof(SMAcutionRegisterError)){
                        const auto smARE = ServerMsg::conv<SMAcutionRegisterError>(buf, bufSize);
                        switch(smARE.error){
                            case ACUTIONREGERR_BADITEM : m_runProc->addCBLog(CBLOG_ERR, u8"寄售物品无效"); return;
                            case ACUTIONREGERR_BADPRICE: m_runProc->addCBLog(CBLOG_ERR, u8"寄售价格无效"); return;
                            case ACUTIONREGERR_BADNOTE : m_runProc->addCBLog(CBLOG_ERR, u8"寄售留言无效"); return;
                            default: break;
                        }
                    }
                    m_runProc->addCBLog(CBLOG_ERR, u8"寄售失败");
                    return;
                }
            default:
                {
                    setPending(false);
                    m_runProc->addCBLog(CBLOG_ERR, u8"寄售失败");
                    return;
                }
        }
    });
}

void AcutionRegisterBoard::confirmCancel()
{
    if(m_pending){
        return;
    }

    auto inputBoardPtr = dynamic_cast<InputStringBoard *>(m_runProc->getWidget("InputStringBoard"));
    fflassert(inputBoardPtr);
    inputBoardPtr->waitInput(u8"<layout><par>你确定取消吗？</par></layout>", false, [this](std::u8string)
    {
        closeRegister(false);
    });
}

void AcutionRegisterBoard::restoreGrabbedItem()
{
    auto &invPack = m_runProc->getMyHero()->getInvPack();
    if(const auto grabbedItem = invPack.getGrabbedItem()){
        invPack.add(grabbedItem);
        invPack.setGrabbedItem({});
    }
}

void AcutionRegisterBoard::restoreOwnedItem()
{
    if(m_item){
        m_runProc->getMyHero()->getInvPack().add(m_item);
        m_item = {};
        m_price = 0;
    }
}

void AcutionRegisterBoard::closeRegister(bool registered)
{
    if(!registered){
        restoreOwnedItem();
    }
    else{
        m_item = {};
    }
    restoreGrabbedItem();

    m_price = 0;
    m_dragging = false;
    m_noteBoard.clear();
    m_noteBoard.setFocus(false);
    setPending(false);

    if(auto invBoardPtr = dynamic_cast<InventoryBoard *>(m_runProc->getWidget("InventoryBoard"))){
        invBoardPtr->setShow(false);
    }
    if(auto inputBoardPtr = dynamic_cast<InputStringBoard *>(m_runProc->getWidget("InputStringBoard"))){
        inputBoardPtr->setShow(false);
    }

    setShow(false);
    setFocus(false);
}
