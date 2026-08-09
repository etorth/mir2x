#include <algorithm>
#include <tuple>
#include <type_traits>
#include <utility>
#include "colorf.hpp"
#include "dbcomid.hpp"
#include "hero.hpp"
#include "gui/friendchatboard/friendchatboard.hpp"
#include "labelboard.hpp"
#include "layoutboard.hpp"
#include "pngtexdb.hpp"
#include "pngtexoffdb.hpp"
#include "processrun.hpp"
#include "sdldevice.hpp"
#include "strf.hpp"
#include "uidf.hpp"
#include "herostateboard.hpp"

extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern PNGTexOffDB *g_equipDB;
extern SDLDevice *g_sdlDevice;

HeroStateBoard::HeroStateBoard(HeroStateBoard::InitArgs args)
    : Widget
      {{
          .dir = DIR_NONE,
          .x = []{ return g_sdlDevice->getRendererWidth () / 2; },
          .y = []{ return g_sdlDevice->getRendererHeight() / 2; },
          .parent = std::move(args.parent),
      }}
    , m_gridList([this]()
      {
          std::remove_cvref_t<decltype(this->m_gridList)> gridList;
          gridList.fill({});

          gridList[WLG_DRESS] = WearGrid
          {
              .x = m_equipCharX,
              .y = m_equipCharY - 100,
              .w = 60,
              .h = 110,
              .type = u8"衣服",
          };

          gridList[WLG_HELMET] = WearGrid
          {
              .x = m_equipCharX + 10,
              .y = m_equipCharY - 135,
              .w = 30,
              .h = 25,
              .type = u8"头盔",
          };

          gridList[WLG_WEAPON] = WearGrid
          {
              .x = m_equipCharX - 50,
              .y = m_equipCharY - 120,
              .w = 45,
              .h = 90,
              .type = u8"武器",
          };

          gridList[WLG_SHOES] = WearGrid
          {
              .x = 20,
              .y = 268,
              .w = 43,
              .h = 61,
              .type = u8"鞋",
          };

          gridList[WLG_NECKLACE] = WearGrid{.x = 221, .y =  94, .w = 43, .h = 43, .type = u8"项链"};
          gridList[WLG_ARMRING0] = WearGrid{.x =  20, .y = 184, .w = 43, .h = 43, .type = u8"手镯"};
          gridList[WLG_ARMRING1] = WearGrid{.x = 222, .y = 184, .w = 43, .h = 43, .type = u8"手镯"};
          gridList[WLG_RING0   ] = WearGrid{.x =  20, .y = 224, .w = 43, .h = 43, .type = u8"戒指"};
          gridList[WLG_RING1   ] = WearGrid{.x = 222, .y = 224, .w = 43, .h = 43, .type = u8"戒指"};
          gridList[WLG_TORCH   ] = WearGrid{.x = 100, .y = 294, .w = 42, .h = 43, .type = u8"火把"};
          gridList[WLG_CHARM   ] = WearGrid{.x = 140, .y = 294, .w = 42, .h = 43, .type = u8"魅力|护身符"};

          return gridList;
      }())

    , m_closeButton
      {{
          .x = 249,
          .y = 312,
          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },
          .onTrigger = [this](Widget *, int){ close(); },
          .parent{this},
      }}

    , m_tradeButton
      {{
          .dir = DIR_UPLEFT,
          .x = 208,
          .y = 15,
          .textFunc = to_cstr(u8"交易申请"),
          .onTrigger = [this](Widget *, int){ requestTrade(); },
          .attrs
          {
              .active = [this]{ return target() != nullptr; },
          },
          .parent{this},
      }}

    , m_friendButton
      {{
          .dir = DIR_UPLEFT,
          .x = 208,
          .y = 30,
          .textFunc = to_cstr(u8"加为好友"),
          .onTrigger = [this](Widget *, int)
          {
              if(const auto hero = target()){
                  dynamic_cast<FriendChatBoard *>(m_processRun->getWidget("FriendChatBoard"))->requestAddFriend(SDChatPeer
                  {
                      .id = hero->dbid(),
                      .name = hero->getName(),
                      .despvar = SDChatPeerPlayerVar
                      {
                          .gender = hero->gender(),
                          .job = hero->job(),
                      },
                  }, false);
              }
          },
          .attrs
          {
              .active = [this]{ return target() != nullptr; },
          },
          .parent{this},
      }}

    , m_processRun(fflcheck(args.runProc))
{
    setShow(false);
    if(auto texPtr = g_progUseDB->retrieve(0X00002020)){
        setW(SDLDeviceHelper::getTextureWidth (texPtr));
        setH(SDLDeviceHelper::getTextureHeight(texPtr));
    }
    else{
        throw fflpanic("no valid hero status board frame texture");
    }
}

void HeroStateBoard::updateDefault(double)
{
    if(show() && !target()){
        close();
    }
}

void HeroStateBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    if(auto texPtr = g_progUseDB->retrieve(0X00002020)){
        g_sdlDevice->drawTexture(texPtr, m.x, m.y);
    }

    const auto hero = target();
    if(hero){
        if(auto [texPtr, dx, dy] = g_equipDB->retrieve(hero->gender() ? 0X00000000 : 0X00000001); texPtr){
            g_sdlDevice->drawTexture(texPtr, m.x + m_equipCharX + dx, m.y + m_equipCharY + dy);
        }

        LabelBoard{{.label = to_u8rawstr(hero->getName()).c_str(), .font{.color = hero->getNameColor() | 0XFF}}}.draw({.dir=DIR_NONE, .x=m.x + 142, .y=m.y + 38});
        if(const auto dressItemID = hero->getWLItem(WLG_DRESS).itemID){
            if(const auto dressGfxID = DBCOM_ITEMRECORD(dressItemID).pkgGfxID; dressGfxID >= 0){
                if(auto [texPtr, dx, dy] = g_equipDB->retrieve(to_u32(dressGfxID) | 0X01000000); texPtr){
                    g_sdlDevice->drawTexture(texPtr, m.x + m_equipCharX + dx, m.y + m_equipCharY + dy);
                }
            }
        }

        if(const auto weaponItemID = hero->getWLItem(WLG_WEAPON).itemID){
            if(const auto useGfxIndex = DBCOM_ITEMRECORD(weaponItemID).shape; useGfxIndex > 0){
                if(auto [texPtr, dx, dy] = g_equipDB->retrieve(0X01000000 + DBCOM_ITEMRECORD(weaponItemID).pkgGfxID); texPtr){
                    g_sdlDevice->drawTexture(texPtr, m.x + m_equipCharX + dx, m.y + m_equipCharY + dy);
                }
            }
        }

        if(const auto helmetItemID = hero->getWLItem(WLG_HELMET).itemID){
            if(const auto useGfxIndex = DBCOM_ITEMRECORD(helmetItemID).shape; useGfxIndex > 0){
                if(auto [texPtr, dx, dy] = g_equipDB->retrieve(0X01000000 + DBCOM_ITEMRECORD(helmetItemID).pkgGfxID); texPtr){
                    g_sdlDevice->drawTexture(texPtr, m.x + m_equipCharX + dx, m.y + m_equipCharY + dy);
                }
            }
        }
        else if(hero->getWLDesp().hair >= HAIR_BEGIN){
            if(auto [texPtr, dx, dy] = g_equipDB->retrieve((hero->gender() ? 0X0000003C : 0X00000046) + hero->getWLDesp().hair - HAIR_BEGIN); texPtr){
                SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, hero->getWLDesp().hairColor);
                g_sdlDevice->drawTexture(texPtr, m.x + m_equipCharX + dx, m.y + m_equipCharY + dy);
            }
        }

        for(size_t i = WLG_W_BEGIN; i < WLG_W_END; ++i){
            if(const auto &item = hero->getWLItem(i)){
                if(auto texPtr = g_itemDB->retrieve(DBCOM_ITEMRECORD(item.itemID).pkgGfxID | 0X01000000)){
                    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
                    const int dstX = m.x + m_gridList[i].x + (m_gridList[i].w - texW) / 2;
                    const int dstY = m.y + m_gridList[i].y + (m_gridList[i].h - texH) / (i == WLG_SHOES ? 1 : 2);
                    g_sdlDevice->drawTexture(texPtr, dstX, dstY);
                }
            }
        }
    }

    if(hero){
        const auto [mouseX, mouseY] = SDLDeviceHelper::getMousePLoc();
        for(size_t i = WLG_BEGIN; i < WLG_END; ++i){
            if(mathf::pointInRectangle(mouseX, mouseY, m.x + m_gridList[i].x, m.y + m_gridList[i].y, m_gridList[i].w, m_gridList[i].h)){
                if(i >= WLG_W_BEGIN && i < WLG_W_END){
                    const auto [texID, dx, dy] = [i]() -> std::tuple<uint32_t, int, int>
                    {
                        if(i == WLG_SHOES){
                            return {0X06000002, -1, -6};
                        }
                        else{
                            return {0X06000001, -1, -3};
                        }
                    }();

                    if(auto texPtr = g_progUseDB->retrieve(texID)){
                        SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, colorf::WHITE + colorf::A_SHF(128));
                        g_sdlDevice->drawTexture(texPtr, m.x + m_gridList[i].x + dx, m.y + m_gridList[i].y + dy);
                    }
                }
                drawItemHoverText(i);
                break;
            }
        }
    }

    drawChild(&m_tradeButton , m);
    drawChild(&m_friendButton, m);
    drawChild(&m_closeButton , m);
}

bool HeroStateBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_closeButton.processEventParent(event, valid, m)){
        return true;
    }

    if(m_tradeButton.processEventParent(event, valid, m)){
        return true;
    }

    if(m_friendButton.processEventParent(event, valid, m)){
        return true;
    }

    switch(event.type){
        case SDL_EVENT_MOUSE_MOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(to_d(event.motion.x), to_d(event.motion.y)) || focus())){
                    if(const auto par = parent()){
                        moveBy(to_d(event.motion.xrel), to_d(event.motion.yrel), par->roi());
                    }
                    else{
                        moveBy(to_d(event.motion.xrel), to_d(event.motion.yrel), Widget::makeROI(0, 0, g_sdlDevice->getRendererSize()));
                    }
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                if(event.button.button == SDL_BUTTON_LEFT){
                    return consumeFocus(m.in(to_d(event.button.x), to_d(event.button.y)));
                }
                return consumeFocus(false);
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

void HeroStateBoard::inspect(uint64_t uid)
{
    if(!uidf::isPlayer(uid) || uid == m_processRun->getMyHeroUID()){
        return;
    }

    m_targetUID = uid;
    if(const auto hero = target()){
        hero->getName();
        m_processRun->queryPlayerWLDesp(uid);

        flipShow(true);
        setFocus(true);
    }
    else{
        m_targetUID = 0;
    }
}

void HeroStateBoard::close()
{
    m_targetUID = 0;
    flipShow(false);
    setFocus(false);
}

Hero *HeroStateBoard::target() const
{
    return dynamic_cast<Hero *>(m_processRun->findUID(m_targetUID));
}

void HeroStateBoard::requestTrade()
{
    if(m_targetUID){
        m_processRun->requestDirectTrade(m_targetUID);
    }
}

void HeroStateBoard::drawItemHoverText(int wltype) const
{
    const auto hero = target();
    if(!hero){
        return;
    }

    const auto &item = hero->getWLItem(wltype);
    if(!item){
        return;
    }

    const auto &ir = DBCOM_ITEMRECORD(item.itemID);
    fflassert(ir);

    const LayoutBoard hoverTextBoard
    {{
        .lineWidth = 200,

        .initXML = to_cstr(item.getXMLLayout().c_str()),
        .lineAlign = LALIGN_JUSTIFY,
    }};

    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
    const auto textBoxW = std::max<int>(hoverTextBoard.w(), 200) + 20;
    const auto textBoxH = hoverTextBoard.h() + 20;

    g_sdlDevice->fillRectangle(colorf::RGBA(  0,   0,   0, 200), mousePX, mousePY, textBoxW, textBoxH, 5);
    g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 200), mousePX, mousePY, textBoxW, textBoxH, 5);
    hoverTextBoard.draw({.x=mousePX + 10, .y=mousePY + 10});
}
