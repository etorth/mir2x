#include <algorithm>
#include <utility>
#include "strf.hpp"
#include "xmlf.hpp"
#include "colorf.hpp"
#include "client.hpp"
#include "dbcomid.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "inputstringboard.hpp"
#include "gui/friendchatboard/friendchatboard.hpp"
#include "auctionregisterboard.hpp"
#include "auctionboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern Client *g_client;

AuctionBoard::AuctionBoard(ProcessRun *argProc, Widget *argParent, bool argAutoDelete)
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

    , m_category
      {{
          .dir = DIR_NONE,
          .x = 61,
          .y = 20,
          .textFunc = [this]() -> std::string
          {
              switch(m_itemList.getItemList().category){
                  case AUCTIONCAT_ALL     : return to_cstr(u8"所有物品");
                  case AUCTIONCAT_DRESS   : return to_cstr(u8"衣服");
                  case AUCTIONCAT_WEAPON  : return to_cstr(u8"武器");
                  case AUCTIONCAT_NECKLACE: return to_cstr(u8"项链");
                  case AUCTIONCAT_HELMET  : return to_cstr(u8"头盔");
                  case AUCTIONCAT_RING    : return to_cstr(u8"戒指");
                  case AUCTIONCAT_ARMRING : return to_cstr(u8"手镯");
                  case AUCTIONCAT_SHOES   : return to_cstr(u8"鞋类");
                  case AUCTIONCAT_POTION  : return to_cstr(u8"药品");
                  case AUCTIONCAT_BOOK    : return to_cstr(u8"图书");
                  case AUCTIONCAT_OTHER   : return to_cstr(u8"其他物品");
                  default                 : return to_cstr(u8"寄售");
              }
          },
          .font
          {
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_itemCount
      {{
          .dir = DIR_NONE,
          .x = 240,
          .y = 20,
          .textFunc = [this]() -> std::string
          {
              return to_cstr(str_printf(u8"共%zu件", m_itemList.size()));
          },
          .font
          {
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_itemRange
      {{
          .dir = DIR_NONE,
          .x = 419,
          .y = 20,
          .textFunc = [this]() -> std::string
          {
              if(const auto firstIndex = m_itemList.firstIndex(); firstIndex.has_value()){
                  const auto lastIndex = std::min<size_t>(firstIndex.value() + AuctionItemList::m_rowCount, m_itemList.size());
                  return to_cstr(str_printf(u8"第%zu-%zu件物品", firstIndex.value() + 1, lastIndex));
              }
              return {};
          },
          .font
          {
              .color = colorf::YELLOW_A255,
          },
          .attrs
          {
              .show = [this]{ return !m_itemList.empty(); },
          },
          .parent{this},
      }}

    , m_columnItem
      {{
          .dir = DIR_NONE,
          .x = 64,
          .y = 62,
          .textFunc = to_cstr(u8"物品"),
          .font
          {
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_columnSeller
      {{
          .dir = DIR_NONE,
          .x = 188,
          .y = 62,
          .textFunc = to_cstr(u8"寄售人"),
          .font
          {
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_columnTime
      {{
          .dir = DIR_NONE,
          .x = 293,
          .y = 62,
          .textFunc = to_cstr(u8"剩余"),
          .font
          {
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_columnPrice
      {{
          .dir = DIR_NONE,
          .x = 399,
          .y = 62,
          .textFunc = to_cstr(u8"价格"),
          .font
          {
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_itemDetailUpper
      {{
          .x = 486,
          .y = 23,

          .onContactSeller = [this](const SDChatPeer &seller)
          {
              auto chatBoard = dynamic_cast<FriendChatBoard *>(m_runProc->getWidget("FriendChatBoard"));
              fflassert(chatBoard);

              setFocus(false);
              chatBoard->openChat(seller);
          },

          .parent{this},
      }}

    , m_itemDetailLower
      {{
          .x = 486,
          .y = 168,
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
              m_itemList.movePrev();
          },

          .attrs
          {
              .active = [this]{ return m_itemList.canMovePrev(); },
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
              m_itemList.moveNext();
          },

          .attrs
          {
              .active = [this]{ return m_itemList.canMoveNext(); },
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
              refreshItemList();
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
              // zsdb uses 1228, but auction board frame image uses 1904
              // here use zsdb and off texture to redraw the button off state

              .off  = 0X000000B3,
              .on   = 0X000000B3,
              .down = 0X000000B4,
          },
          .onTrigger = [this](Widget *, int)
          {
              flipShow(false);
              if(auto registerBoardPtr = dynamic_cast<AuctionRegisterBoard *>(m_runProc->getWidget("AuctionRegisterBoard"))){
                  registerBoardPtr->beginRegister();
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
          .onTrigger = [this](Widget *, int)
          {
              confirmBuy();
          },

          .attrs
          {
              .active = [this] -> bool
              {
                  if(m_pending){
                      return false;
                  }

                  if(const auto item = selectedItem()){
                      return item->seller.id != m_runProc->getMyHeroDBID();
                  }

                  return false;
              },
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
          .onTrigger = [this](Widget *, int)
          {
              confirmUnregister();
          },

          .attrs
          {
              .active = [this]
              {
                  if(m_pending){
                      return false;
                  }

                  if(const auto item = selectedItem()){
                      return item->seller.id == m_runProc->getMyHeroDBID();
                  }

                  return false;
              },
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
    setShow(false);
}

bool AuctionBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(const auto oldSelected = m_itemList.selectedIndex(); m_itemList.processEventParent(event, valid, m)){
        if(const auto newSelected = m_itemList.selectedIndex(); oldSelected != newSelected){
            m_itemDetailUpper.setItem(newSelected.has_value() ? m_itemList.indexItem(newSelected.value()) : nullptr);
            m_itemDetailLower.setItem(newSelected.has_value() ? m_itemList.indexItem(newSelected.value()) : nullptr);
        }
        return true;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_itemDetailUpper.show() && m_itemDetailUpper   .processEventParent(event, valid, m)){ return true; }
    if(m_itemDetailLower.show() && m_itemDetailLower   .processEventParent(event, valid, m)){ return true; }
    if(                            m_buttonPrevious    .processEventParent(event, valid, m)){ return true; }
    if(                            m_buttonNext        .processEventParent(event, valid, m)){ return true; }
    if(                            m_buttonRefresh     .processEventParent(event, valid, m)){ return true; }
    if(                            m_buttonItemSearch  .processEventParent(event, valid, m)){ return true; }
    if(                            m_buttonSellerSearch.processEventParent(event, valid, m)){ return true; }
    if(                            m_buttonRegister    .processEventParent(event, valid, m)){ return true; }
    if(                            m_buttonBuy         .processEventParent(event, valid, m)){ return true; }
    if(                            m_buttonCancel      .processEventParent(event, valid, m)){ return true; }
    if(                            m_buttonClose       .processEventParent(event, valid, m)){ return true; }

    switch(event.type){
        case SDL_EVENT_KEY_DOWN:
            {
                if(event.key.key == SDLK_ESCAPE){
                    flipShow(false);
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

void AuctionBoard::setItemList(SDAuctionItemList sdAuctionItemList)
{
    m_itemList.setItemList(std::move(sdAuctionItemList));
    m_itemDetailUpper.setItem(nullptr);
    m_itemDetailLower.setItem(nullptr);
}

const SDAuctionItem *AuctionBoard::selectedItem() const
{
    const auto selectedIndex = m_itemList.selectedIndex();
    return selectedIndex.has_value() ? m_itemList.indexItem(selectedIndex.value()) : nullptr;
}

void AuctionBoard::refreshItemList()
{
    const auto currCategory = m_itemList.getItemList().category;
    const auto queryCategory = (currCategory >= AUCTIONCAT_BEGIN && currCategory < AUCTIONCAT_END) ? currCategory : AUCTIONCAT_ALL;

    g_client->send({CM_QUERYAUCTIONITEMLIST, CMQueryAuctionItemList
    {
        .category = to_u8(queryCategory),
    }});
}

void AuctionBoard::confirmBuy()
{
    if(m_pending){
        return;
    }

    const auto item = selectedItem();
    if(!item){
        m_runProc->addCBLog(CBLOG_ERR, u8"请先选择要购买的寄售物品");
        return;
    }

    if(item->seller.id == m_runProc->getMyHeroDBID()){
        m_runProc->addCBLog(CBLOG_ERR, u8"不能购买自己寄售的物品");
        return;
    }

    if(item->price > m_runProc->getMyHero()->getGold()){
        m_runProc->addCBLog(CBLOG_ERR, u8"金币不够");
        return;
    }

    const auto &ir = DBCOM_ITEMRECORD(item->item.itemID);
    fflassert(ir);

    auto inputBoard = dynamic_cast<InputStringBoard *>(m_runProc->getWidget("InputStringBoard"));
    fflassert(inputBoard);

    std::string prompt = "<layout>";
    prompt += xmlf::toParString("你确定花费%s金币购买%s吗？", str_ksep(item->price).c_str(), to_cstr(ir.name));
    prompt += xmlf::toParString("%s", to_cstr(u8"购买成功后，物品会通过系统投递发送。"));
    prompt += "</layout>";

    inputBoard->waitChoice(to_u8rawstr(prompt), [auctionID = item->auctionID, itemName = std::string(to_cstr(ir.name)), this]
    {
        buyItem(auctionID, itemName);
    });
}

void AuctionBoard::buyItem(uint64_t auctionID, std::string itemName)
{
    if(m_pending){
        return;
    }

    m_pending = true;
    g_client->send({CM_BUYAUCTIONITEM, CMBuyAuctionItem
    {
        .auctionID = auctionID,
    }},

    [itemName = std::move(itemName), this](uint8_t headCode, const uint8_t *buf, size_t bufSize)
    {
        m_pending = false;
        switch(headCode){
            case SM_OK:
                {
                    m_runProc->addCBLog(CBLOG_SYS, u8"%s购买成功，物品已发送到系统投递", itemName.c_str());
                    refreshItemList();
                    return;
                }
            case SM_AUCTIONBUYERROR:
                {
                    switch(ServerMsg::conv<SMAuctionBuyError>(buf, bufSize).error){
                        case AUCTIONBUYERR_UNAVAILABLE:
                            {
                                m_runProc->addCBLog(CBLOG_ERR, u8"该寄售物品已售出或到期");
                                refreshItemList();
                                return;
                            }
                        case AUCTIONBUYERR_INSUFFICIENT:
                            {
                                m_runProc->addCBLog(CBLOG_ERR, u8"金币不够");
                                return;
                            }
                        case AUCTIONBUYERR_OWNITEM:
                            {
                                m_runProc->addCBLog(CBLOG_ERR, u8"不能购买自己寄售的物品");
                                return;
                            }
                        case AUCTIONBUYERR_BADITEM:
                        default:
                            {
                                m_runProc->addCBLog(CBLOG_ERR, u8"购买失败");
                                return;
                            }
                    }
                }
            default:
                {
                    m_runProc->addCBLog(CBLOG_ERR, u8"购买失败");
                    return;
                }
        }
    });
}

void AuctionBoard::confirmUnregister()
{
    if(m_pending){
        return;
    }

    const auto item = selectedItem();
    if(!item){
        m_runProc->addCBLog(CBLOG_ERR, u8"请先选择要下架的寄售物品");
        return;
    }

    if(item->seller.id != m_runProc->getMyHeroDBID()){
        m_runProc->addCBLog(CBLOG_ERR, u8"只能下架自己寄售的物品");
        return;
    }

    const auto &ir = DBCOM_ITEMRECORD(item->item.itemID);
    fflassert(ir);

    auto inputBoard = dynamic_cast<InputStringBoard *>(m_runProc->getWidget("InputStringBoard"));
    fflassert(inputBoard);

    std::string prompt = "<layout>";
    prompt += xmlf::toParString("你确定下架%s吗？", to_cstr(ir.name));
    prompt += xmlf::toParString("%s", to_cstr(u8"下架后，物品会通过系统投递退回。"));
    prompt += "</layout>";

    inputBoard->waitChoice(to_u8rawstr(prompt), [auctionID = item->auctionID, itemName = std::string(to_cstr(ir.name)), this]
    {
        unregisterItem(auctionID, itemName);
    });
}

void AuctionBoard::unregisterItem(uint64_t auctionID, std::string itemName)
{
    if(m_pending){
        return;
    }

    m_pending = true;
    g_client->send({CM_UNREGISTERAUCTIONITEM, CMUnregisterAuctionItem
    {
        .auctionID = auctionID,
    }},

    [itemName = std::move(itemName), this](uint8_t headCode, const uint8_t *buf, size_t bufSize)
    {
        m_pending = false;
        switch(headCode){
            case SM_OK:
                {
                    m_runProc->addCBLog(CBLOG_SYS, u8"%s已下架，物品已发送到系统投递", itemName.c_str());
                    refreshItemList();
                    return;
                }
            case SM_AUCTIONUNREGISTERERROR:
                {
                    switch(ServerMsg::conv<SMAuctionUnregisterError>(buf, bufSize).error){
                        case AUCTIONUNREGERR_UNAVAILABLE:
                            {
                                m_runProc->addCBLog(CBLOG_ERR, u8"该寄售物品已售出或下架");
                                refreshItemList();
                                return;
                            }
                        case AUCTIONUNREGERR_BADITEM:
                        default:
                            {
                                m_runProc->addCBLog(CBLOG_ERR, u8"下架失败");
                                return;
                            }
                    }
                }
            default:
                {
                    m_runProc->addCBLog(CBLOG_ERR, u8"下架失败");
                    return;
                }
        }
    });
}
