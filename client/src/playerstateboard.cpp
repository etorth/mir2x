#include <type_traits>
#include "strf.hpp"
#include "uidf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "combatnode.hpp"
#include "processrun.hpp"
#include "pngtexoffdb.hpp"
#include "soundeffectdb.hpp"
#include "inventoryboard.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern PNGTexOffDB *g_equipDB;
extern SoundEffectDB *g_seffDB;
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
          {},

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
          {SYS_U32NIL, 0X0000001C, 0X0000001D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              setShow(false);
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
    setShow(false);
    if(auto texPtr = g_progUseDB->retrieve(0X06000000)){
        std::tie(m_w, m_h) = SDLDeviceHelper::getTextureSize(texPtr);
    }
    else{
        throw fflerror("no valid player status board frame texture");
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

    const auto fnDrawLabel = [this](int labelX, int labelY, const std::u8string &s, uint32_t color = colorf::WHITE)
    {
        LabelBoard(DIR_UPLEFT, 0, 0, s.c_str(), 1, 12, 0, colorf::maskRGB(color) + colorf::A_SHF(255)).drawAt(DIR_NONE, x() + labelX, y() + labelY);
    };

    const auto myHeroPtr = m_processRun->getMyHero();
    fnDrawLabel(279, 97 + 24 * 0, str_printf(u8"%d", to_d(myHeroPtr->getLevel())));
    fnDrawLabel(279, 97 + 24 * 1, str_printf(u8"%.2f%%", myHeroPtr->getLevelRatio() * 100.0));

    const bool hasHealth = myHeroPtr->getSDHealth().has_value();
    fnDrawLabel(279, 97 + 24 * 2, str_printf(u8"%d/%d", hasHealth ? myHeroPtr->getSDHealth().value().hp : 0, hasHealth ? myHeroPtr->getSDHealth().value().getMaxHP() : 0));
    fnDrawLabel(279, 97 + 24 * 3, str_printf(u8"%d/%d", hasHealth ? myHeroPtr->getSDHealth().value().mp : 0, hasHealth ? myHeroPtr->getSDHealth().value().getMaxMP() : 0));

    const auto combatNode = myHeroPtr->getCombatNode();
    const auto invPackWeight = myHeroPtr->getInvPack().getWeight();
    const auto invPackWeightColor = (invPackWeight > combatNode.load.inventory) ? colorf::RED : colorf::WHITE;
    fnDrawLabel(279, 97 + 24 * 4, str_printf(u8"%d/%d", invPackWeight, combatNode.load.inventory), invPackWeightColor);

    const auto bodyLoad = [myHeroPtr]()
    {
        int result = 0;
        for(int i = WLG_BEGIN; i < WLG_END; ++i){
            if(i == WLG_WEAPON){
                continue;
            }

            const auto &item = myHeroPtr->getWLItem(i);
            if(!item){
                continue;
            }

            const auto &ir = DBCOM_ITEMRECORD(item.itemID);
            fflassert(ir);
            result += ir.weight;
        }
        return result;
    }();

    const auto bodyLoadColor = (bodyLoad > combatNode.load.body) ? colorf::RED : colorf::WHITE;
    fnDrawLabel(279, 97 + 24 * 5, str_printf(u8"%d/%d", bodyLoad, combatNode.load.body), bodyLoadColor);

    const auto weaponLoad = [myHeroPtr]()
    {
        const auto &item = myHeroPtr->getWLItem(WLG_WEAPON);
        if(!item){
            return 0;
        }

        const auto &ir = DBCOM_ITEMRECORD(item.itemID);
        fflassert(ir);
        return ir.weight;
    }();

    const auto weaponLoadColor = (weaponLoad > combatNode.load.weapon) ? colorf::RED : colorf::WHITE;
    fnDrawLabel(279, 97 + 24 * 6, str_printf(u8"%d/%d", weaponLoad, combatNode.load.weapon), weaponLoadColor);
    fnDrawLabel(279, 97 + 24 * 7, str_printf(u8"%d", combatNode.dcHit));
    fnDrawLabel(279, 97 + 24 * 8, str_printf(u8"%d", combatNode.dcDodge));

    LabelBoard(DIR_UPLEFT, 0, 0, str_printf(u8"攻击 %d - %d", combatNode. dc[0], combatNode. dc[1]).c_str(), 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, x() +  21, y() + 317);
    LabelBoard(DIR_UPLEFT, 0, 0, str_printf(u8"防御 %d - %d", combatNode. ac[0], combatNode. ac[1]).c_str(), 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, x() + 130, y() + 317);
    LabelBoard(DIR_UPLEFT, 0, 0, str_printf(u8"魔法 %d - %d", combatNode. mc[0], combatNode. mc[1]).c_str(), 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, x() +  21, y() + 345);
    LabelBoard(DIR_UPLEFT, 0, 0, str_printf(u8"魔防 %d - %d", combatNode.mac[0], combatNode.mac[1]).c_str(), 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, x() + 130, y() + 345);
    LabelBoard(DIR_UPLEFT, 0, 0, str_printf(u8"道术 %d - %d", combatNode. sc[0], combatNode. sc[1]).c_str(), 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, x() + 233, y() + 345);

    LabelBoard(DIR_UPLEFT, 0, 0, u8"攻击元素", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, x() + 10, y() + 376);
    LabelBoard(DIR_UPLEFT, 0, 0, u8"防御元素", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, x() + 10, y() + 406);
    LabelBoard(DIR_UPLEFT, 0, 0, u8"弱点元素", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, x() + 10, y() + 436);

    for(int i = MET_BEGIN; i < MET_END; ++i){
        const auto [dcElem, acElem] = [i, &combatNode]() -> std::array<int, 2>
        {
            switch(i){
                case MET_FIRE   : return {combatNode.dcElem.fire   , combatNode.acElem.fire   };
                case MET_ICE    : return {combatNode.dcElem.ice    , combatNode.acElem.ice    };
                case MET_LIGHT  : return {combatNode.dcElem.light  , combatNode.acElem.light  };
                case MET_WIND   : return {combatNode.dcElem.wind   , combatNode.acElem.wind   };
                case MET_HOLY   : return {combatNode.dcElem.holy   , combatNode.acElem.holy   };
                case MET_DARK   : return {combatNode.dcElem.dark   , combatNode.acElem.dark   };
                case MET_PHANTOM: return {combatNode.dcElem.phantom, combatNode.acElem.phantom};
                default         : return {0, 0};
            }
        }();

        fflassert(dcElem >= 0);
        const int elemGridX = x() + 62 + (i - MET_BEGIN) * 37;
        const int elemGridY[]
        {
            y() + 374 + 0 * 30,
            y() + 374 + 1 * 30,
            y() + 374 + 2 * 30,
        };

        if(dcElem > 0){
            g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X06000010 + to_u32(i - MET_BEGIN)), elemGridX, elemGridY[0]);
            LabelBoard(DIR_UPLEFT, 0, 0, str_printf(u8"%+d", dcElem).c_str(), 1, 12, 0, colorf::GREEN + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, elemGridX + 20, elemGridY[0] + 1);
        }

        if(acElem > 0){
            g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X06000010 + to_u32(i - MET_BEGIN)), elemGridX, elemGridY[1]);
            LabelBoard(DIR_UPLEFT, 0, 0, str_printf(u8"%+d", acElem).c_str(), 1, 12, 0, colorf::GREEN + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, elemGridX + 20, elemGridY[1] + 1);
        }
        else if(acElem < 0){
            g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X06000020 + to_u32(i - MET_BEGIN)), elemGridX, elemGridY[2]);
            LabelBoard(DIR_UPLEFT, 0, 0, str_printf(u8"%+d", acElem).c_str(), 1, 12, 0, colorf::RED   + colorf::A_SHF(255)).drawAt(DIR_UPLEFT, elemGridX + 20, elemGridY[2] + 1);
        }
    }

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

    const int labelGridX = 220;
    const int labelGridY =  87;
    const int labelGridW =  18;
    const int labelGridH =  18;
    const int labelGridD =  24;
    constexpr const char8_t * labelName[]
    {
        u8"等级",
        u8"经验",
        u8"生命值",
        u8"魔法值",
        u8"背包负重",
        u8"身体负重",
        u8"武器负重",
        u8"命中",
        u8"躲避",
    };

    for(int i = 0; i < 9; ++i){
        if(mathf::pointInRectangle(mouseX, mouseY, x() + labelGridX, y() + labelGridY + labelGridD * i, labelGridW, labelGridH)){
            g_sdlDevice->fillRectangle(colorf::RGBA(0, 100, 0, 100), x() + labelGridX, y() + labelGridY + labelGridD * i, labelGridW, labelGridH);
            const LabelBoard labelNameBoard(DIR_UPLEFT, 0, 0, labelName[i], 1, 12, 0, colorf::WHITE + colorf::A_SHF(255));
            g_sdlDevice->fillRectangle(colorf::RGBA(0, 100, 0, 200), mouseX - labelNameBoard.w(), mouseY - labelNameBoard.h(), labelNameBoard.w(), labelNameBoard.h());
            labelNameBoard.drawAt(DIR_DOWNRIGHT, mouseX, mouseY);
        }
    }

    if(g_clientArgParser->debugPlayerStateBoard){
        for(size_t i = WLG_BEGIN; i < WLG_END; ++i){
            g_sdlDevice->drawRectangle(colorf::BLUE + colorf::A_SHF(255), x() + m_gridList[i].x, y() + m_gridList[i].y, m_gridList[i].w, m_gridList[i].h);
        }
    }
    m_closeButton.draw();
}

bool PlayerStateBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
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
                    return consumeFocus(true);
                }
                return consumeFocus(false);
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
                            return consumeFocus(in(event.button.x, event.button.y));
                        }
                    default:
                        {
                            return consumeFocus(false);
                        }
                }
            }
        default:
            {
                return consumeFocus(false);
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

    const LayoutBoard hoverTextBoard
    {
        DIR_UPLEFT,
        0,
        0,
        200,

        to_cstr(item.getXMLLayout().c_str()),
        0,

        {},
        false,
        false,
        false,
        false,

        1,
        12,
        0,
        colorf::WHITE + colorf::A_SHF(255),
        0,

        LALIGN_JUSTIFY,
    };

    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
    const auto textBoxW = std::max<int>(hoverTextBoard.w(), 200) + 20;
    const auto textBoxH = hoverTextBoard.h() + 20;

    g_sdlDevice->fillRectangle(colorf::RGBA(  0,   0,   0, 200), mousePX, mousePY, textBoxW, textBoxH, 5);
    g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 200), mousePX, mousePY, textBoxW, textBoxH, 5);
    hoverTextBoard.drawAt(DIR_UPLEFT, mousePX + 10, mousePY + 10);
}
