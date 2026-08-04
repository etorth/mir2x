#include <algorithm>
#include <charconv>
#include <limits>
#include "strf.hpp"
#include "utf8f.hpp"
#include "xmlf.hpp"
#include "client.hpp"
#include "dbcomid.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "inventoryboard.hpp"
#include "inputstringboard.hpp"
#include "acutionregisterboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern Client *g_client;

AcutionRegisterBoard::AcutionRegisterBoard(ProcessRun *argProc, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .dir = DIR_NONE,
          .x = []{ return g_sdlDevice->getRendererWidth () / 2; },
          .y = []{ return g_sdlDevice->getRendererHeight() / 2; },
          .w = std::nullopt,
          .h = std::nullopt,
          .parent{argParent, argAutoDelete},
      }}

    , m_runProc(fflcheck(argProc))
    , m_background
      {{
          .texLoadFunc = [](const Widget *) -> SDL_Texture *
          {
              return g_progUseDB->retrieve(0X00001410);
          },
          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , m_note
      {{
          .x = 9,
          .y = 68,
          .enableIME = [this]
          {
              return m_runProc->getRuntimeConfig<RTCFG_IME>();
          },
          .parent{this},
      }}

    , m_item
      {{
          .x = 241,
          .y = 45,
          .onClick = [this]
          {
              auto &invPack = m_runProc->getMyHero()->getInvPack();
              auto &grabbedItem = invPack.getGrabbedItem();

              if(grabbedItem){ // has item in hand
                  restoreOwnedItem();
                  m_item.setItem(grabbedItem);
                  invPack.setGrabbedItem({});
                  m_price.clear();
                  m_note.setInputFocus(true);
              }
              else if(m_item.item()){
                  invPack.setGrabbedItem(m_item.takeItem());
                  m_price.clear();
              }
          },
          .parent{this},
      }}

    , m_price
      {{
          .x = 25,
          .y = 232,
          .onClick = [this]
          {
              if(m_item.item()){
                  setPrice();
              }
          },
          .parent{this},
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
    setShow(false);
}

void AcutionRegisterBoard::beginRegister()
{
    auto invBoard = dynamic_cast<InventoryBoard *>(m_runProc->getWidget("InventoryBoard"));

    fflassert(invBoard);
    invBoard->flipShow(true);

    flipShow(true);
    setFocus(true);
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

    if(Widget::processEventDefault(event, valid, m)){
        return true;
    }

    if(m_pending
            && event.type == SDL_EVENT_MOUSE_BUTTON_DOWN
            && event.button.button == SDL_BUTTON_LEFT){
        const auto inButton = [&m, &event, this](const Widget &button)
        {
            return m.create(button.roi(this)).in(to_d(event.button.x), to_d(event.button.y));
        };

        if(inButton(m_buttonRegister) || inButton(m_buttonCancel) || inButton(m_buttonClose)){
            return consumeFocus(true);
        }
    }

    switch(event.type){
        case SDL_EVENT_KEY_DOWN:
            {
                return consumeFocus(false);
            }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                m_dragging = false;
                if(event.button.button == SDL_BUTTON_LEFT){
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
    m_note.setInputEnabled(!pending);
    m_item.setInputEnabled(!pending);
    m_price.setInputEnabled(!pending);
    m_buttonRegister.setActive(!pending);
    m_buttonCancel.setActive(!pending);
    m_buttonClose.setActive(!pending);

    if(auto invBoard = dynamic_cast<InventoryBoard *>(m_runProc->getWidget("InventoryBoard"))){
        invBoard->setActive(!pending);
    }
}

void AcutionRegisterBoard::setPrice()
{
    auto inputBoard = dynamic_cast<InputStringBoard *>(m_runProc->getWidget("InputStringBoard"));
    fflassert(inputBoard);

    inputBoard->waitInput(u8"<layout><par>你想卖多少钱呢?</par></layout>", false, [this](std::u8string input)
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
        m_price.setPrice(price);
    });
}

void AcutionRegisterBoard::confirmRegister()
{
    if(m_pending){
        return;
    }

    if(!m_item.item()){
        m_runProc->addCBLog(CBLOG_ERR, u8"请先放入寄售物品");
        return;
    }

    if(m_price.price() == 0 || m_price.price() > to_u64(std::numeric_limits<int64_t>::max())){
        m_runProc->addCBLog(CBLOG_ERR, u8"请先设置有效的寄售价格");
        return;
    }

    const std::string note = m_note.getText();
    if(!utf8f::valid(note)){
        m_runProc->addCBLog(CBLOG_ERR, u8"寄售留言不是有效的UTF-8文本");
        return;
    }

    if(note.size() > SYS_ACUTIONNOTESIZE){
        m_runProc->addCBLog(CBLOG_ERR, u8"寄售留言不能超过%zu字节", SYS_ACUTIONNOTESIZE);
        return;
    }

    const auto &ir = DBCOM_ITEMRECORD(m_item.item().itemID);
    fflassert(ir);

    auto inputBoard = dynamic_cast<InputStringBoard *>(m_runProc->getWidget("InputStringBoard"));
    fflassert(inputBoard);

    std::u8string prompt = to_u8rawstr("<layout>"
            + xmlf::toParString("你确定上架%s吗?", to_cstr(ir.name))
            + xmlf::toParString("委托交易将会扣除手续费500，交易成功后扣除2%%交易额")
            + "</layout>");

    inputBoard->waitInput(std::move(prompt), false, [this](std::u8string)
    {
        registerItem();
    });
}

void AcutionRegisterBoard::registerItem()
{
    if(m_pending || !m_item.item()){
        return;
    }

    const std::string note = m_note.getText();
    if(!utf8f::valid(note)){
        m_runProc->addCBLog(CBLOG_ERR, u8"寄售留言不是有效的UTF-8文本");
        return;
    }

    if(note.size() > SYS_ACUTIONNOTESIZE){
        m_runProc->addCBLog(CBLOG_ERR, u8"寄售留言不能超过%zu字节", SYS_ACUTIONNOTESIZE);
        return;
    }

    if(m_price.price() == 0 || m_price.price() > to_u64(std::numeric_limits<int64_t>::max())){
        m_runProc->addCBLog(CBLOG_ERR, u8"寄售价格无效");
        return;
    }

    const auto &item = m_item.item();
    const auto &ir = DBCOM_ITEMRECORD(item.itemID);
    fflassert(ir);
    const std::string itemName = to_cstr(ir.name);

    CMRegisterAcutionItem cmRAI
    {
        .itemID = item.itemID,
        .seqID = item.seqID,
        .price = m_price.price(),
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

    auto inputBoard = dynamic_cast<InputStringBoard *>(m_runProc->getWidget("InputStringBoard"));
    fflassert(inputBoard);
    inputBoard->waitInput(u8"<layout><par>你确定取消吗？</par></layout>", false, [this](std::u8string)
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
    if(m_item.item()){
        const auto item = m_item.takeItem();
        m_runProc->getMyHero()->getInvPack().add(item);
        m_price.clear();
    }
}

void AcutionRegisterBoard::closeRegister(bool registered)
{
    if(!registered){
        restoreOwnedItem();
    }
    else{
        m_item.clear();
    }
    restoreGrabbedItem();

    m_price.clear();
    m_dragging = false;
    m_note.clear();
    m_note.setInputFocus(false);
    setPending(false);

    if(auto invBoard = dynamic_cast<InventoryBoard *>(m_runProc->getWidget("InventoryBoard"))){
        invBoard->flipShow(false);
    }

    if(auto inputBoard = dynamic_cast<InputStringBoard *>(m_runProc->getWidget("InputStringBoard"))){
        inputBoard->flipShow(false);
    }

    flipShow(false);
    setFocus(false);
}
