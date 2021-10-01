/*
 * =====================================================================================
 *
 *       Filename: playerstateboard.cpp
 *        Created: 10/08/2017 19:22:30
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include <type_traits>
#include "strf.hpp"
#include "uidf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "combatnode.hpp"
#include "processrun.hpp"
#include "dbcomrecord.hpp"
#include "pngtexoffdb.hpp"
#include "inventoryboard.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern PNGTexOffDB *g_equipDB;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

PlayerStateBoard::PlayerStateBoard(int argX, int argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          DIR_UPLEFT,
          argX,
          argY,
          0,
          0,

          widgetPtr,
          autoDelete
      }
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
              .x = 10,
              .y = 240,
              .h = 56,
              .type = u8"鞋",
          };

          gridList[WLG_NECKLACE] = WearGrid{.x = 168, .y =  88, .type = u8"项链"};
          gridList[WLG_ARMRING0] = WearGrid{.x =  10, .y = 155, .type = u8"手镯"};
          gridList[WLG_ARMRING1] = WearGrid{.x = 168, .y = 155, .type = u8"手镯"};
          gridList[WLG_RING0   ] = WearGrid{.x =  10, .y = 195, .type = u8"戒指"};
          gridList[WLG_RING1   ] = WearGrid{.x = 168, .y = 195, .type = u8"戒指"};
          gridList[WLG_TORCH   ] = WearGrid{.x =  88, .y = 265, .type = u8"火把"};
          gridList[WLG_CHARM   ] = WearGrid{.x = 128, .y = 265, .type = u8"魅力|护身符"};

          return gridList;
      }())

    , m_closeButton
      {
          DIR_UPLEFT,
          288,
          13,
          {SYS_TEXNIL, 0X0000001C, 0X0000001D},

          nullptr,
          nullptr,
          [this]()
          {
              show(false);
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
          false,
      }
    , m_processRun(runPtr)
{
    show(false);
    if(auto texPtr = g_progUseDB->retrieve(0X06000000)){
        std::tie(m_w, m_h) = SDLDeviceHelper::getTextureSize(texPtr);
    }
    else{
        throw fflerror("no valid player status board frame texture");
    }

    for(int r: {0, 1, 2}){
        for(int i = 0; i < 7; ++i){
            m_elemStateList.push_back(new TritexButton
            {
                DIR_UPLEFT,
                62  + i * 37,
                374 + r * 30,

                {
                    SYS_TEXNIL,
                    0X06000010 + to_u32(i),
                    0X06000020 + to_u32(i),
                },

                nullptr,
                nullptr,
                nullptr,

                0,
                0,
                0,
                0,

                false,
                true,
                this,
                true,
            });
        }
    }
}

void PlayerStateBoard::update(double)
{
}

void PlayerStateBoard::drawEx(int, int, int, int, int, int) const
{
    if(auto texPtr = g_progUseDB->retrieve(0X06000000)){
        g_sdlDevice->drawTexture(texPtr, x(), y());
    }

    const auto myHeroPtr = m_processRun->getMyHero();
    const auto combatNode = myHeroPtr->getCombatNode();

    LabelBoard(DIR_UPLEFT, 0, 0, str_printf(u8"攻击 %d - %d", combatNode. dc[0], combatNode. dc[1]).c_str(), 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, x() +  21, y() + 317);
    LabelBoard(DIR_UPLEFT, 0, 0, str_printf(u8"防御 %d - %d", combatNode. ac[0], combatNode. ac[1]).c_str(), 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, x() + 130, y() + 317);
    LabelBoard(DIR_UPLEFT, 0, 0, str_printf(u8"魔法 %d - %d", combatNode.mdc[0], combatNode.mdc[1]).c_str(), 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, x() +  21, y() + 345);
    LabelBoard(DIR_UPLEFT, 0, 0, str_printf(u8"魔防 %d - %d", combatNode.mac[0], combatNode.mac[1]).c_str(), 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, x() + 130, y() + 345);
    LabelBoard(DIR_UPLEFT, 0, 0, str_printf(u8"道术 %d - %d", combatNode.sdc[0], combatNode.sdc[1]).c_str(), 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, x() + 233, y() + 345);

    if(auto [texPtr, dx, dy] = g_equipDB->retrieve(myHeroPtr->gender() ? 0X00000000 : 0X00000001); texPtr){
        g_sdlDevice->drawTexture(texPtr, x() + m_equipCharX + dx, y() + m_equipCharY + dy);
    }

    LabelBoard(DIR_UPLEFT, 0, 0, to_u8cstr(myHeroPtr->getName()), 1, 12, 0, myHeroPtr->getNameColor() | 0XFF).drawAt(DIR_NONE, x() + 164, y() + 38);
    if(const auto dressItemID = myHeroPtr->getWLItem(WLG_DRESS).itemID){
        if(const auto dressGfxID = DBCOM_ITEMRECORD(dressItemID).pkgGfxID; dressGfxID >= 0){
            if(auto [texPtr, dx, dy] = g_equipDB->retrieve(to_u32(dressGfxID) | 0X01000000); texPtr){
                g_sdlDevice->drawTexture(texPtr, x() + m_equipCharX + dx, y() + m_equipCharY + dy);
            }
        }
    }

    if(const auto weaponItemID = myHeroPtr->getWLItem(WLG_WEAPON).itemID){
        if(const auto useGfxIndex = DBCOM_ITEMRECORD(weaponItemID).shape; useGfxIndex > 0){
            if(auto [texPtr, dx, dy] = g_equipDB->retrieve(0X01000000 + DBCOM_ITEMRECORD(weaponItemID).pkgGfxID); texPtr){
                g_sdlDevice->drawTexture(texPtr, x() + m_equipCharX + dx, y() + m_equipCharY + dy);
            }
        }
    }

    if(const auto helmetItemID = myHeroPtr->getWLItem(WLG_HELMET).itemID){
        if(const auto useGfxIndex = DBCOM_ITEMRECORD(helmetItemID).shape; useGfxIndex > 0){
            if(auto [texPtr, dx, dy] = g_equipDB->retrieve(0X01000000 + DBCOM_ITEMRECORD(helmetItemID).pkgGfxID); texPtr){
                g_sdlDevice->drawTexture(texPtr, x() + m_equipCharX + dx, y() + m_equipCharY + dy);
            }
        }
    }
    else{
        if(myHeroPtr->getWLDesp().hair >= HAIR_BEGIN){
            if(auto [texPtr, dx, dy] = g_equipDB->retrieve((myHeroPtr->gender() ? 0X0000003C : 0X00000046) + myHeroPtr->getWLDesp().hair - HAIR_BEGIN); texPtr){
                SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, myHeroPtr->getWLDesp().hairColor);
                g_sdlDevice->drawTexture(texPtr, x() + m_equipCharX + dx, y() + m_equipCharY + dy);
            }
        }
    }

    for(size_t i = WLG_W_BEGIN; i < WLG_W_END; ++i){
        if(const auto &item = m_processRun->getMyHero()->getWLItem(i)){
            if(auto texPtr = g_itemDB->retrieve(DBCOM_ITEMRECORD(item.itemID).pkgGfxID | 0X01000000)){
                const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
                const int dstX = x() + m_gridList[i].x + (m_gridList[i].w - texW) / 2;
                const int dstY = y() + m_gridList[i].y + (m_gridList[i].h - texH) / (i == WLG_SHOES ? 1 : 2);
                g_sdlDevice->drawTexture(texPtr, dstX, dstY);
            }
        }
    }

    m_closeButton.draw();
    for(auto buttonPtr: m_elemStateList){
        buttonPtr->draw();
    }

    const auto [mouseX, mouseY] = SDLDeviceHelper::getMousePLoc();
    for(size_t i = WLG_BEGIN; i < WLG_END; ++i){
        if(mathf::pointInRectangle(mouseX, mouseY, x() + m_gridList[i].x, y() + m_gridList[i].y, m_gridList[i].w, m_gridList[i].h)){
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
                    g_sdlDevice->drawTexture(texPtr, x() + m_gridList[i].x + dx, y() + m_gridList[i].y + dy);
                }
            }
            drawItemHoverText(i);
            break;
        }
    }

    if(g_clientArgParser->debugPlayerStateBoard){
        for(size_t i = WLG_BEGIN; i < WLG_END; ++i){
            g_sdlDevice->drawRectangle(colorf::BLUE + colorf::A_SHF(255), x() + m_gridList[i].x, y() + m_gridList[i].y, m_gridList[i].w, m_gridList[i].h);
        }
    }
}

bool PlayerStateBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return focusConsume(this, false);
    }

    if(!show()){
        return focusConsume(this, false);
    }

    bool consumed = false;
    for(auto buttonPtr: m_elemStateList){
        consumed |= buttonPtr->processEvent(event, !consumed && valid);
    }

    if(consumed){
        return focusConsume(this, false);
    }

    if(m_closeButton.processEvent(event, valid)){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (in(event.motion.x, event.motion.y) || focus())){
                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, x() + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, y() + event.motion.yrel));
                    moveBy(newX - x(), newY - y());
                    return focusConsume(this, true);
                }
                return focusConsume(this, false);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                switch(event.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            auto myHeroPtr = m_processRun->getMyHero();
                            auto &invPackRef = myHeroPtr->getInvPack();
                            for(size_t i = WLG_BEGIN; i < WLG_END; ++i){
                                if(mathf::pointInRectangle(event.button.x, event.button.y, x() + m_gridList[i].x, y() + m_gridList[i].y, m_gridList[i].w, m_gridList[i].h)){
                                    if(const auto grabbedItem = invPackRef.getGrabbedItem()){
                                        if(myHeroPtr->canWear(grabbedItem.itemID, i)){
                                            m_processRun->requestEquipWear(grabbedItem.itemID, grabbedItem.seqID, i);
                                        }
                                        else{
                                            invPackRef.add(grabbedItem);
                                            invPackRef.setGrabbedItem({});
                                        }
                                    }
                                    else if(myHeroPtr->getWLItem(i)){
                                        m_processRun->requestGrabWear(i);
                                    }
                                    break;
                                }
                            }
                            return focusConsume(this, in(event.button.x, event.button.y));
                        }
                    default:
                        {
                            return focusConsume(this, false);
                        }
                }
            }
        default:
            {
                return focusConsume(this, false);
            }
    }
}

void PlayerStateBoard::drawItemHoverText(int wltype) const
{
    const auto &item = m_processRun->getMyHero()->getWLItem(wltype);
    if(!item){
        return;
    }

    const auto &ir = DBCOM_ITEMRECORD(item.itemID);
    fflassert(ir);

    LayoutBoard hoverTextBoard
    {
        DIR_UPLEFT,
        0,
        0,
        200,

        false,
        {0, 0, 0, 0},

        false,

        1,
        12,
        0,
        colorf::WHITE + colorf::A_SHF(255),
        0,

        LALIGN_JUSTIFY,
    };

    hoverTextBoard.loadXML(to_cstr(item.getXMLLayout().c_str()));
    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
    const auto textBoxW = std::max<int>(hoverTextBoard.w(), 200) + 20;
    const auto textBoxH = hoverTextBoard.h() + 20;

    g_sdlDevice->fillRectangle(colorf::RGBA(0, 0,   0, 200), mousePX, mousePY, textBoxW, textBoxH, 5);
    g_sdlDevice->drawRectangle(colorf::RGBA(0, 0, 255, 255), mousePX, mousePY, textBoxW, textBoxH, 5);
    hoverTextBoard.drawAt(DIR_UPLEFT, mousePX + 10, mousePY + 10);
}
