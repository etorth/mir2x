#include <algorithm>
#include <charconv>
#include <limits>
#include <utility>
#include "colorf.hpp"
#include "invpack.hpp"
#include "layoutboard.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "directtradeboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

DirectTradeBoard::DirectTradeBoard(DirectTradeBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),
          .x = std::move(args.x),
          .y = std::move(args.y),
          .w = std::nullopt,
          .h = std::nullopt,
          .parent = std::move(args.parent),
      }}

    , m_runProc(fflcheck(args.runProc))

    , m_background
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00002010); },
          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , m_localItemList
      {{
          .x = 264,
          .y = 47,
          .onClick = [this](DirectTradeItemList::ClickEvent event)
          {
              onLocalItemClick(event);
          },
          .parent{this},
      }}

    , m_peerItemList
      {{
          .x = 13,
          .y = 47,
          .parent{this},
      }}

    , m_localName
      {{
          .dir = DIR_NONE,
          .x = 429,
          .y = 18,
          .textFunc = [this]
          {
              return m_runProc->getMyHeroChatPeer().name;
          },
          .font
          {
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_localState
      {{
          .dir = DIR_NONE,
          .x = 319,
          .y = 18,
          .textFunc = [this]
          {
              if(m_localConfirmed){
                  return to_cstr(u8"已确认交易");
              }
              if(m_localLockPending){
                  return to_cstr(u8"锁定中");
              }
              return m_localLocked ? to_cstr(u8"已锁定物品") : to_cstr(u8"编辑中");
          },
          .font
          {
              .color = [this]
              {
                  if(m_localConfirmed || m_localLockPending){
                      return colorf::YELLOW_A255;
                  }
                  return m_localLocked ? colorf::GREEN_A255 : colorf::GREY_A255;
              },
          },
          .parent{this},
      }}

    , m_peerState
      {{
          .dir = DIR_NONE,
          .x = 180,
          .y = 18,
          .textFunc = [this]
          {
              if(m_peerConfirmed){
                  return to_cstr(u8"已确认交易");
              }
              return m_peerLocked ? to_cstr(u8"已锁定物品") : to_cstr(u8"编辑中");
          },
          .font
          {
              .color = [this]
              {
                  if(m_peerConfirmed){
                      return colorf::YELLOW_A255;
                  }
                  return m_peerLocked ? colorf::GREEN_A255 : colorf::GREY_A255;
              },
          },
          .parent{this},
      }}

    , m_peerNameBoard
      {{
          .dir = DIR_NONE,
          .x = 67,
          .y = 18,
          .textFunc = [this]{ return m_peerName; },
          .font
          {
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_localGoldInput
      {{
          .x = 333,
          .y = 289,
          .w = 96,
          .h = 18,
          .font
          {
              .color = colorf::YELLOW_A255,
          },
          .onChange = [this](std::string)
          {
              m_localConfirmed = false;
              syncLocalOffer(false);
          },
          .validate = [](const std::string &currentInput, const std::string &newInput)
          {
              return currentInput.size() + newInput.size() <= 10
                  && newInput.find_first_not_of("0123456789") == std::string::npos;
          },
          .parent{this},
      }}

    , m_peerGoldBoard
      {{
          .dir = DIR_NONE,
          .x = 134,
          .y = 298,
          .textFunc = [this]{ return str_ksep(m_peerGold, ','); },
          .font
          {
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_buttonTrade
      {{
          .x = 227,
          .y = 278,
          .texIDFunc = [this](int state) -> std::optional<uint32_t>
          {
              if(m_localLocked){
                  return (state == BEVENT_DOWN) ? 0X00000191 : 0X00000190;
              }
              return (state == BEVENT_DOWN) ? 0X000000B6 : 0X000000B5;
          },
          .onTrigger = [this](Widget *, int)
          {
              // One button drives both protocol phases. Lock first freezes the
              // offer; the next click is enabled only after both sides locked.
              if(!m_localLocked){
                  if(syncLocalOffer(true)){
                      m_localLockPending = true;
                      m_localGoldInput.setFocus(false);
                  }
                  return;
              }

              m_localConfirmed = true;
              m_runProc->commitDirectTrade(m_peerUID);
          },
          .attrs
          {
              .active = [this]
              {
                  if(m_localLocked){
                      return !m_localConfirmed && m_peerLocked;
                  }
                  return !m_localLockPending && parsedLocalGold().has_value();
              },
          },
          .parent{this},
      }}

    , m_buttonClose
      {{
          .x = 454,
          .y = 282,
          .texIDList
          {
              .off  = 0X0000001C,
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },
          .onTrigger = [this](Widget *, int)
          {
              close();
          },
          .parent{this},
      }}
{
    m_localGoldInput.setInput("0");
    m_localGoldInput.setActive([this]{ return !m_localLocked && !m_localLockPending; });

    setShow(false);
    setSize([this]{ return m_background.w(); },
            [this]{ return m_background.h(); });
}

void DirectTradeBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    Widget::drawDefault(m);

    if(const auto item = m_localItemList.hoveredItem()){
        drawItemHoverText(*item);
    }
    else if(const auto item = m_peerItemList.hoveredItem()){
        drawItemHoverText(*item);
    }
}

bool DirectTradeBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE){
        close();
        return true;
    }

    if(Widget::processEventDefault(event, valid, m)){
        return true;
    }

    switch(event.type){
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                m_dragging = event.button.button == SDL_BUTTON_LEFT && m.in(to_d(event.button.x), to_d(event.button.y));
                return consumeFocus(m_dragging);
            }
        case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                return std::exchange(m_dragging, false) ? consumeFocus(m.in(to_d(event.button.x), to_d(event.button.y))) : false;
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
                return false;
            }
        default:
            {
                return false;
            }
    }
}

void DirectTradeBoard::begin(uint64_t peerUID, std::string peerName)
{
    restoreLocalItems();
    m_peerItemList.clear();

    m_peerUID = peerUID;
    m_peerName = std::move(peerName);
    m_peerGold = 0;
    m_localLocked = false;
    m_localLockPending = false;
    m_peerLocked = false;
    m_localConfirmed = false;
    m_peerConfirmed = false;

    m_localGoldInput.setInput("0");

    flipShow(true);
    setFocus(true);
}

void DirectTradeBoard::close(bool notify)
{
    if(notify && m_peerUID){
        m_runProc->cancelDirectTrade(m_peerUID);
    }

    // The board owns only a provisional client-side copy of offered items.
    // If a racing server commit already won, the following SM_INVENTORY replaces
    // this restored pack with the authoritative post-trade inventory.
    restoreLocalItems();
    m_peerItemList.clear();

    m_dragging = false;
    m_localLocked = false;
    m_localLockPending = false;
    m_peerLocked = false;
    m_localConfirmed = false;
    m_peerConfirmed = false;
    m_peerGold = 0;
    m_peerUID = 0;
    m_peerName.clear();

    m_localGoldInput.setInput("0");
    m_localGoldInput.setFocus(false);

    flipShow(false);
    setFocus(false);
}

void DirectTradeBoard::complete()
{
    if(auto hero = m_runProc->getMyHero()){
        hero->getInvPack().setGrabbedItem({});
    }

    // Completion arrives after the authoritative inventory and gold updates.
    // Do not return the offered items to InvPack: they were consumed by trade.
    m_localItemList.clear();
    m_peerItemList.clear();

    m_dragging = false;
    m_localLocked = false;
    m_localLockPending = false;
    m_peerLocked = false;
    m_localConfirmed = false;
    m_peerConfirmed = false;
    m_peerGold = 0;
    m_peerUID = 0;
    m_peerName.clear();

    m_localGoldInput.setInput("0");
    m_localGoldInput.setFocus(false);

    flipShow(false);
    setFocus(false);
}

uint32_t DirectTradeBoard::localGold() const
{
    return parsedLocalGold().value_or(0);
}

void DirectTradeBoard::setLocalLocked(bool locked)
{
    m_localLocked = locked;
    m_localLockPending = false;
    if(!locked){
        m_localConfirmed = false;
    }
    else{
        m_localGoldInput.setFocus(false);
    }
}

void DirectTradeBoard::setPeerOffer(SDDirectTradeOffer offer)
{
    m_peerGold = offer.gold;
    m_peerItemList.setItemList(std::move(offer.itemList));
    m_peerLocked = offer.locked;
    m_peerConfirmed = offer.confirmed;
}

void DirectTradeBoard::applyLocalOfferAck(const SDDirectTradeOffer &offer)
{
    // Ignore delayed echoes for an older edit. In particular, the UI must not
    // become locked until the server has accepted the exact visible offer.
    if(!localOfferMatches(offer)){
        return;
    }

    if(offer.locked){
        setLocalLocked(true);
        m_localConfirmed = offer.confirmed;
    }
    else if(!m_localLockPending){
        setLocalLocked(false);
    }
}

void DirectTradeBoard::rejectLocalOffer()
{
    m_localLockPending = false;
    m_localConfirmed = false;
}

std::optional<uint32_t> DirectTradeBoard::parsedLocalGold() const
{
    const auto input = m_localGoldInput.getRawString();
    if(input.empty()){
        return 0;
    }

    if(input.size() > 10 || !std::ranges::all_of(input, [](unsigned char ch){ return std::isdigit(ch); })){
        return {};
    }

    uint64_t value = 0;
    const auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), value);
    if(ec != std::errc() || ptr != input.data() + input.size() || value > std::numeric_limits<uint32_t>::max()){
        return {};
    }

    if(const auto hero = m_runProc->getMyHero(); hero && value > hero->getGold()){
        return {};
    }
    return to_u32(value);
}

bool DirectTradeBoard::localOfferMatches(const SDDirectTradeOffer &offer) const
{
    const auto gold = parsedLocalGold();
    const auto itemList = m_localItemList.itemList();
    if(!gold || offer.gold != gold.value() || offer.itemList.size() != itemList.size()){
        return false;
    }

    for(size_t i = 0; i < itemList.size(); ++i){
        const auto &lhs = itemList.at(i);
        const auto &rhs = offer.itemList.at(i);
        if(lhs.itemID != rhs.itemID || lhs.seqID != rhs.seqID || lhs.count != rhs.count){
            return false;
        }
    }
    return true;
}

bool DirectTradeBoard::syncLocalOffer(bool locked)
{
    if(!m_peerUID){
        return false;
    }

    const auto gold = parsedLocalGold();
    if(!gold){
        return false;
    }

    return m_runProc->updateDirectTradeOffer(m_peerUID, gold.value(), locked, m_localItemList.itemList());
}

void DirectTradeBoard::onLocalItemClick(DirectTradeItemList::ClickEvent event)
{
    if(m_localLocked || m_localLockPending){
        return;
    }

    auto hero = fflcheck(m_runProc->getMyHero());
    auto &invPack = hero->getInvPack();
    const auto grabbedItem = invPack.getGrabbedItem();

    if(grabbedItem){
        if(event.packBinIndex >= 0){
            const auto selectedBin = m_localItemList.takeItem(to_uz(event.packBinIndex));
            if(!m_localItemList.addItem(grabbedItem, selectedBin.x, selectedBin.y)){
                fflassert(m_localItemList.addItem(selectedBin.item, selectedBin.x, selectedBin.y));
                m_runProc->addCBLog(CBLOG_ERR, u8"该物品过大，无法放入交易栏");
                return;
            }
            invPack.setGrabbedItem(selectedBin.item);
        }
        else{
            const auto [gridW, gridH] = InvPack::getPackBinSize(grabbedItem.itemID);
            if(!m_localItemList.addItem(grabbedItem, event.gridX - gridW / 2, event.gridY - gridH / 2)){
                m_runProc->addCBLog(CBLOG_ERR, u8"该物品过大，无法放入交易栏");
                return;
            }
            invPack.setGrabbedItem({});
        }

        m_localConfirmed = false;
        syncLocalOffer(false);
        return;
    }

    if(event.packBinIndex >= 0){
        invPack.setGrabbedItem(m_localItemList.takeItem(to_uz(event.packBinIndex)).item);
        m_localConfirmed = false;
        syncLocalOffer(false);
    }
}

void DirectTradeBoard::restoreLocalItems()
{
    auto itemList = m_localItemList.takeItemList();
    if(itemList.empty()){
        return;
    }

    if(auto hero = m_runProc->getMyHero()){
        auto &invPack = hero->getInvPack();
        for(auto &item: itemList){
            invPack.add(std::move(item), false);
        }
    }
}

void DirectTradeBoard::drawItemHoverText(const SDItem &item) const
{
    const LayoutBoard hoverTextBoard
    {{
        .lineWidth = 200,
        .initXML = to_cstr(item.getXMLLayout().c_str()),
        .lineAlign = LALIGN_JUSTIFY,
    }};

    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
    const int textBoxW = std::max<int>(hoverTextBoard.w(), 200) + 20;
    const int textBoxH = hoverTextBoard.h() + 20;

    const int drawBoardPX = mathf::bound<int>(mousePX, 0, g_sdlDevice->getRendererWidth () - textBoxW);
    const int drawBoardPY = mathf::bound<int>(mousePY, 0, g_sdlDevice->getRendererHeight() - textBoxH);

    g_sdlDevice->fillRectangle(colorf::RGBA(0, 0, 0, 200), drawBoardPX, drawBoardPY, textBoxW, textBoxH, 5);
    g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 200), drawBoardPX, drawBoardPY, textBoxW, textBoxH, 5);
    hoverTextBoard.draw({.x = drawBoardPX + 10, .y = drawBoardPY + 10});
}
