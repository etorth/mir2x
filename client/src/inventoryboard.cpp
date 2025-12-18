#include "luaf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "soundeffectdb.hpp"
#include "processrun.hpp"
#include "inventoryboard.hpp"

extern PNGTexDB *g_progUseDB;
extern PNGTexDB *g_itemDB;
extern SoundEffectDB *g_seffDB;
extern SDLDevice *g_sdlDevice;

InventoryBoard::InventoryBoard(InventoryBoard::InitArgs args)
    : Widget
      {{
          .x = args.x,
          .y = args.y,

          .parent = std::move(args.parent),
      }}

    , m_processRun(args.runProc)

    , m_wmdAniBoard
      {
          DIR_UPLEFT,
          23,
          14,
          this,
      }

    , m_slider
      {{
          .bar
          {
              .x = 410,
              .y = 63,
              .w = 8,
              .h = 369,
              .v = true,
          },

          .index = 0,
          .parent{this},
      }}

    , m_sortButton
      {{
          .x = 374,
          .y = 12,

          .texIDList
          {
              .on   = 0X000000C0,
              .down = 0X000000C1,
          },

          .onTrigger = [this](Widget *, int)
          {
              m_processRun->getMyHero()->getInvPack().repack();
          },

          .parent{this},
      }}

    , m_closeButton
      {{
          .x = 394,
          .y = 498,

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](Widget *, int)
          {
              setShow(false);
              m_sdInvOp.clear();
          },

          .parent{this},
      }}

    , m_invOpButton
      {{
          .x = m_invOpButtonX + 3,
          .y = m_invOpButtonY + 3,

          .texIDList
          {
              .off  = 0X000000B3, // use trade gfx, needs it to setup widget size
              .on   = 0X000000B3,
              .down = 0X000000B4,
          },

          .onTrigger = [this](Widget *, int)
          {
              commitInvOp();
          },

          .parent{this},
      }}

    , m_invOpTitle
      {{
          .dir = DIR_NONE,
          .x = 238,
          .y = 25,

          .textFunc = [this] -> std::string
          {
              switch(m_sdInvOp.invOp){
                  case INVOP_NONE  : return "【背包】";
                  case INVOP_TRADE : return "【请选择出售物品】";
                  case INVOP_SECURE: return "【请选择存储物品】";
                  case INVOP_REPAIR: return "【请选择修理物品】";
                  default: return {};
              }
          },

          .font
          {
              .id = 1,
              .size = 12,
          },

          .parent{this},
      }}

    , m_goldStr
      {{
          .dir = DIR_NONE,
          .x = 132,
          .y = 486,

          .textFunc = [this] -> std::string
          {
              return str_ksep([this]() -> int
              {
                  if(auto p = m_processRun->getMyHero()){
                      return p->getGold();
                  }
                  return 0;
              }(), ',');
          },

          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
          },

          .parent{this},
      }}
{
    setShow(false);
    auto texPtr = g_progUseDB->retrieve(0X0000001B);
    if(!texPtr){
        throw fflerror("no valid inventory frame texture");
    }

    setW(SDLDeviceHelper::getTextureWidth (texPtr));
    setH(SDLDeviceHelper::getTextureHeight(texPtr));
}

void InventoryBoard::drawItem(int dstX, int dstY, size_t startRow, const PackBin &bin, uint32_t fillColor) const
{
    if(true
            && bin
            && bin.x >= 0
            && bin.y >= 0
            && bin.w >  0
            && bin.h >  0){

        if(auto texPtr = g_itemDB->retrieve(DBCOM_ITEMRECORD(bin.item.itemID).pkgGfxID | 0X01000000)){
            const int startX = dstX + m_invGridX0;
            const int startY = dstY + m_invGridY0 - startRow * SYS_INVGRIDPH;
            const int  viewX = dstX + m_invGridX0;
            const int  viewY = dstY + m_invGridY0;

            const auto [itemPW, itemPH] = SDLDeviceHelper::getTextureSize(texPtr);
            int drawDstX = startX + bin.x * SYS_INVGRIDPW + (bin.w * SYS_INVGRIDPW - itemPW) / 2;
            int drawDstY = startY + bin.y * SYS_INVGRIDPH + (bin.h * SYS_INVGRIDPH - itemPH) / 2;
            int drawSrcX = 0;
            int drawSrcY = 0;
            int drawSrcW = itemPW;
            int drawSrcH = itemPH;

            if(mathf::cropROI(
                        &drawSrcX, &drawSrcY,
                        &drawSrcW, &drawSrcH,
                        &drawDstX, &drawDstY,

                        drawSrcW,
                        drawSrcH,

                        0, 0, -1, -1,
                        viewX, viewY, SYS_INVGRIDPW * SYS_INVGRIDGW, SYS_INVGRIDPH * SYS_INVGRIDGH)){
                g_sdlDevice->drawTexture(texPtr, drawDstX, drawDstY, drawSrcX, drawSrcY, drawSrcW, drawSrcH);
            }

            int binGridX = bin.x;
            int binGridY = bin.y;
            int binGridW = bin.w;
            int binGridH = bin.h;

            if(mathf::rectangleOverlapRegion<int>(0, startRow, SYS_INVGRIDGW, SYS_INVGRIDGH, binGridX, binGridY, binGridW, binGridH)){
                g_sdlDevice->fillRectangle(fillColor,
                        startX + binGridX * SYS_INVGRIDPW,
                        startY + binGridY * SYS_INVGRIDPH, // startY is for (0, 0), not for (0, startRow)
                        binGridW * SYS_INVGRIDPW,
                        binGridH * SYS_INVGRIDPH);

                if(bin.item.count > 1){
                    const LabelBoard itemCount
                    {{
                        .label = to_u8cstr(std::to_string(bin.item.count)),
                        .font
                        {
                            .id = 1,
                            .size = 10,
                            .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
                        },
                    }};
                    itemCount.draw({.dir=DIR_UPRIGHT, .x{startX + (binGridX + binGridW) * SYS_INVGRIDPW}, .y{startY + binGridY * SYS_INVGRIDPH - 2 /* pixel adjust */}});
                }
            }
        }
    }
}

void InventoryBoard::updateDefault(double fUpdateTime)
{
    m_wmdAniBoard.update(fUpdateTime);
}

void InventoryBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    if(auto texPtr = g_progUseDB->retrieve(0X0000001B)){
        g_sdlDevice->drawTexture(texPtr, m.x, m.y);
    }

    const auto myHeroPtr = m_processRun->getMyHero();
    if(!myHeroPtr){
        return;
    }

    const auto startOffX = m.x - m.ro->x;
    const auto startOffY = m.y - m.ro->y;

    const auto startRow = getStartRow();
    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
    const auto cursorOnIndex = getPackBinIndex(mousePX - startOffX, mousePY - startOffY);
    for(int i = 0; i < std::ssize(myHeroPtr->getInvPack().getPackBinList()); ++i){
        const auto fillColor = [i, cursorOnIndex, this]() -> uint32_t
        {
            if(i == cursorOnIndex){
                return colorf::WHITE + colorf::A_SHF(48);
            }
            else if(m_sdInvOp.invOp != INVOP_NONE && i == m_selectedIndex){
                return colorf::BLUE + colorf::A_SHF(48);
            }
            else{
                return 0;
            }
        }();
        drawItem(m.x, m.y, startRow, myHeroPtr->getInvPack().getPackBinList().at(i), fillColor);
    }

    drawChild(&m_goldStr    , m);
    drawChild(&m_invOpTitle , m);
    drawChild(&m_wmdAniBoard, m);
    drawChild(&m_slider     , m);
    drawChild(&m_closeButton, m);

    if(m_sdInvOp.invOp == INVOP_NONE){
        drawChild(&m_sortButton, m);
    }
    else if(m_selectedIndex >= 0){
        g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X0000B0), m.x + m_invOpButtonX, m.y + m_invOpButtonY);
        drawChild(&m_invOpButton, m);
        if(m_invOpCost >= 0){
            drawInvOpCost();
        }
    }

    if(cursorOnIndex >= 0){
        drawItemHoverText(myHeroPtr->getInvPack().getPackBinList().at(cursorOnIndex));
    }
}

bool InventoryBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_closeButton.processEventParent(event, valid, m)){
        consumeFocus(false);
        return true;
    }

    if(m_slider.processEventParent(event, valid, m)){
        return true;
    }

    if(m_sdInvOp.invOp == INVOP_NONE){
        if(m_sortButton.processEventParent(event, valid, m)){
            return true;
        }
    }
    else{
        if(m_selectedIndex >= 0 && m_invOpButton.processEventParent(event, valid, m)){
            return true;
        }
    }

    const auto startXOff = m.x - m.ro->x;
    const auto startYOff = m.y - m.ro->y;

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_ESCAPE:
                        {
                            setShow(false);
                            setFocus(false);
                            return true;
                        }
                    default:
                        {
                            return consumeFocus(false);
                        }
                }
            }
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(event.motion.x, event.motion.y) || focus())){
                    const auto remapXDiff = m.x - m.ro->x;
                    const auto remapYDiff = m.y - m.ro->y;

                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, remapXDiff + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, remapYDiff + event.motion.yrel));

                    moveBy(newX - remapXDiff, newY - remapYDiff);
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                auto myHeroPtr = m_processRun->getMyHero();
                auto &invPackRef = myHeroPtr->getInvPack();
                auto lastGrabbedItem = invPackRef.getGrabbedItem();

                switch(event.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            if(m.in(event.button.x, event.button.y)){
                                if(m_sdInvOp.invOp == INVOP_NONE){
                                    if(const int selectedPackIndex = getPackBinIndex(event.button.x - startXOff, event.button.y - startYOff); selectedPackIndex >= 0){
                                        auto selectedPackBin = invPackRef.getPackBinList().at(selectedPackIndex);
                                        invPackRef.setGrabbedItem(selectedPackBin.item);
                                        invPackRef.remove(selectedPackBin.item);
                                        if(lastGrabbedItem){
                                            // when swapping
                                            // prefer to use current location to store
                                            invPackRef.add(lastGrabbedItem, selectedPackBin.x, selectedPackBin.y);
                                        }
                                    }
                                    else if(lastGrabbedItem){
                                        const auto [gridX, gridY] = getInvGrid(event.button.x - startXOff, event.button.y - startYOff);
                                        const auto [gridW, gridH] = InvPack::getPackBinSize(lastGrabbedItem.itemID);
                                        const auto startGridX = gridX - gridW / 2; // can give an invalid (x, y)
                                        const auto startGridY = gridY - gridH / 2;
                                        invPackRef.add(lastGrabbedItem, startGridX, startGridY);
                                        invPackRef.setGrabbedItem({});
                                    }
                                }
                                else{
                                    if(const int selectedPackIndex = getPackBinIndex(event.button.x - startXOff, event.button.y - startYOff); selectedPackIndex >= 0){
                                        m_selectedIndex = selectedPackIndex;
                                        const auto &selectedItem = invPackRef.getPackBinList().at(selectedPackIndex);

                                        if(m_sdInvOp.hasType(DBCOM_ITEMRECORD(selectedItem.item.itemID).type)){
                                            m_processRun->sendNPCEvent(m_sdInvOp.uid, {}, m_sdInvOp.queryTag.c_str(), str_printf("%d:%d", to_d(selectedItem.item.itemID), to_d(selectedItem.item. seqID)).c_str());
                                        }
                                        else{
                                            m_processRun->addCBLog(CBLOG_ERR, u8"只能维修%s", to_cstr(typeListString(m_sdInvOp.typeList)));
                                            m_selectedIndex = -1;
                                            m_invOpCost = -1;
                                        }
                                    }
                                    else{
                                        m_selectedIndex = -1;
                                        m_invOpCost = -1;
                                    }
                                }
                                return consumeFocus(true);
                            }
                            return consumeFocus(false);
                        }
                    case SDL_BUTTON_RIGHT:
                        {
                            if(m.in(event.button.x, event.button.y)){
                                if(const int selectedPackIndex = getPackBinIndex(event.button.x - startXOff, event.button.y - startYOff); selectedPackIndex >= 0){
                                    const auto &packBin = invPackRef.getPackBinList().at(selectedPackIndex);
                                    packBinConsume(packBin);
                                }
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
        case SDL_MOUSEWHEEL:
            {
                const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
                if(mathf::pointInRectangle<int>(mousePX, mousePY, m.x + m_invGridX0, m.y + m_invGridY0, SYS_INVGRIDGW * SYS_INVGRIDPW, SYS_INVGRIDGH * SYS_INVGRIDPH)){
                    const auto rowCount = getRowCount();
                    if(rowCount > SYS_INVGRIDGH){
                        m_slider.addValue((event.wheel.y > 0 ? -1.0 : 1.0) / (rowCount - SYS_INVGRIDGH), false);
                    }
                    return consumeFocus(true);
                }
                return false;
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

size_t InventoryBoard::getRowCount() const
{
    const auto &packBinList = m_processRun->getMyHero()->getInvPack().getPackBinList();
    if(packBinList.empty()){
        return 0;
    }

    size_t rowCount = 0;
    for(const auto &bin: packBinList){
        rowCount = std::max<size_t>(rowCount, bin.y + bin.h);
    }
    return rowCount;
}

size_t InventoryBoard::getStartRow() const
{
    const size_t rowCount = getRowCount();
    if(rowCount <= SYS_INVGRIDGH){
        return 0;
    }
    return std::lround((rowCount - SYS_INVGRIDGH) * m_slider.getValue());
}

void InventoryBoard::drawInvOpCost() const
{
    if(m_invOpCost < 0){
        return;
    }

    const LabelBoard queryResultBoard
    {{
        .label = to_u8cstr(str_ksep(m_invOpCost, ',')),
        .font
        {
            .id = 1,
            .size = 12,
            .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
        },
    }};
    // queryResultBoard.drawAt(DIR_NONE, m.x + 132, m.y + 503);
    queryResultBoard.draw({.dir=DIR_NONE, .x=132, .y=503});
}

int InventoryBoard::getPackBinIndex(int locPDX, int locPDY) const
{
    const auto [gridX, gridY] = getInvGrid(locPDX, locPDY);
    if(gridX < 0 || gridY < 0){
        return -1;
    }

    const auto startRow = getStartRow();
    const auto myHeroPtr = m_processRun->getMyHero();

    const auto &packBinList = myHeroPtr->getInvPack().getPackBinList();
    for(int i = 0; i < std::ssize(packBinList); ++i){
        const auto &bin = packBinList.at(i);
        if(mathf::pointInRectangle<int>(gridX, gridY, bin.x, bin.y - startRow, bin.w, bin.h)){
            return i;
        }
    }
    return -1;
}

std::tuple<int, int> InventoryBoard::getInvGrid(int locPDX, int locPDY) const
{
    const int gridPX0 = m_invGridX0;
    const int gridPY0 = m_invGridY0;

    if(!mathf::pointInRectangle<int>(locPDX, locPDY, gridPX0, gridPY0, SYS_INVGRIDGW * SYS_INVGRIDPW, SYS_INVGRIDGH * SYS_INVGRIDPH)){
        return {-1, -1};
    }

    return
    {
        (locPDX - gridPX0) / SYS_INVGRIDPW,
        (locPDY - gridPY0) / SYS_INVGRIDPH,
    };
}

void InventoryBoard::drawItemHoverText(const PackBin &bin) const
{
    const LayoutBoard hoverTextBoard
    {{
        .lineWidth = 200,
        .initXML = to_cstr(bin.item.getXMLLayout().c_str()),

        .font
        {
            .id = 1,
            .size = 12,
        },

        .lineAlign = LALIGN_JUSTIFY,
    }};

    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
    const auto textBoxW = std::max<int>(hoverTextBoard.w(), 200) + 20;
    const auto textBoxH = hoverTextBoard.h() + 20;

    const auto drawBoardPX = mathf::bound<int>(mousePX, 0, g_sdlDevice->getRendererWidth () - textBoxW);
    const auto drawBoardPY = mathf::bound<int>(mousePY, 0, g_sdlDevice->getRendererHeight() - textBoxH);

    g_sdlDevice->fillRectangle(colorf::RGBA(  0,   0,   0, 200), drawBoardPX, drawBoardPY, textBoxW, textBoxH, 5);
    g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 200), drawBoardPX, drawBoardPY, textBoxW, textBoxH, 5);
    hoverTextBoard.draw({.x=drawBoardPX + 10, .y=drawBoardPY + 10});
}

void InventoryBoard::packBinConsume(const PackBin &bin)
{
    const auto &ir = DBCOM_ITEMRECORD(bin.item.itemID);
    fflassert(ir);

    // play item consuming sound effect without server confirmation
    // because for some items like 药水, server won't send back confirmation message

    if(false
            || to_u8sv(ir.type) == u8"恢复药水"
            || to_u8sv(ir.type) == u8"强化药水"
            || to_u8sv(ir.type) == u8"技能书"){
        InvPack::playItemSoundEffect(bin.item.itemID, true);
        m_processRun->requestConsumeItem(bin.item.itemID, bin.item.seqID, 1);
    }

    else if(to_u8sv(ir.type) == u8"头盔"){
        InvPack::playItemSoundEffect(bin.item.itemID, true);
        m_processRun->requestEquipWear(bin.item.itemID, bin.item.seqID, WLG_HELMET);
    }

    else if(to_u8sv(ir.type) == u8"武器"){
        InvPack::playItemSoundEffect(bin.item.itemID, true);
        m_processRun->requestEquipWear(bin.item.itemID, bin.item.seqID, WLG_WEAPON);
    }

    else if(to_u8sv(ir.type) == u8"衣服"){
        InvPack::playItemSoundEffect(bin.item.itemID, true);
        m_processRun->requestEquipWear(bin.item.itemID, bin.item.seqID, WLG_DRESS);
    }

    else if(to_u8sv(ir.type) == u8"鞋"){
        InvPack::playItemSoundEffect(bin.item.itemID, true);
        m_processRun->requestEquipWear(bin.item.itemID, bin.item.seqID, WLG_SHOES);
    }

    else if(to_u8sv(ir.type) == u8"项链"){
        InvPack::playItemSoundEffect(bin.item.itemID, true);
        m_processRun->requestEquipWear(bin.item.itemID, bin.item.seqID, WLG_NECKLACE);
    }

    else if(to_u8sv(ir.type) == u8"手镯"){
        InvPack::playItemSoundEffect(bin.item.itemID, true);
        m_processRun->requestEquipWear(bin.item.itemID, bin.item.seqID, WLG_ARMRING0);
    }

    else if(to_u8sv(ir.type) == u8"戒指"){
        InvPack::playItemSoundEffect(bin.item.itemID, true);
        m_processRun->requestEquipWear(bin.item.itemID, bin.item.seqID, WLG_RING0);
    }
}

void InventoryBoard::clearInvOp()
{
    m_sdInvOp.clear();
    m_selectedIndex = -1;
    m_invOpCost = -1;
}

void InventoryBoard::startInvOp(SDStartInvOp sdSIOP)
{
    m_sdInvOp = std::move(sdSIOP);
    switch(m_sdInvOp.invOp){
        case INVOP_TRADE : m_invOpButton.setTexID({0X000000B3, 0X000000B3, 0X000000B4}); return;
        case INVOP_SECURE: m_invOpButton.setTexID({0X000000B5, 0X000000B5, 0X000000B6}); return;
        case INVOP_REPAIR: m_invOpButton.setTexID({0X000000B1, 0X000000B1, 0X000000B2}); return;
        default: throw fflreach();
    }
}

void InventoryBoard::setInvOpCost(int mode, uint32_t itemID, uint32_t seqID, size_t cost)
{
    fflassert(mode >= INVOP_BEGIN);
    fflassert(mode <  INVOP_END);

    if(m_sdInvOp.invOp != mode){
        return;
    }

    if(m_selectedIndex < 0){
        return;
    }

    const auto myHeroPtr = m_processRun->getMyHero();
    const auto &packBinListCRef = myHeroPtr->getInvPack().getPackBinList();

    if(m_selectedIndex >= to_d(packBinListCRef.size())){
        return;
    }

    const auto &binCRef = packBinListCRef.at(m_selectedIndex);
    if(binCRef.item.itemID != itemID || binCRef.item.seqID != seqID){
        return;
    }

    m_invOpCost = to_d(cost);
}

std::u8string InventoryBoard::typeListString(const std::vector<std::u8string> &typeList)
{
    const auto fnConn = []() -> const char8_t *
    {
        const char8_t * connList[]
        {
            u8"和",
            u8"以及",
            u8"或者",
        };

        return connList[std::rand() % std::extent_v<decltype(connList)>];
    };

    switch(typeList.size()){
        case 0:
            {
                return {};
            }
        case 1:
            {
                return typeList[0];
            }
        case 2:
            {
                return typeList[0] + fnConn() + typeList[1];
            }
        default:
            {
                std::u8string result;
                for(size_t i = 0; i < typeList.size() - 2; ++i){
                    result += typeList[i];
                    result += u8"，";
                }
                return result + *(typeList.rbegin() + 1) + fnConn() + *typeList.rbegin();
            }
    }
}

void InventoryBoard::commitInvOp()
{
    if(m_sdInvOp.invOp == INVOP_NONE){
        return;
    }

    if(m_selectedIndex < 0){
        return;
    }

    const auto &selectedItem = m_processRun->getMyHero()->getInvPack().getPackBinList().at(m_selectedIndex);
    if(!selectedItem){
        return;
    }

    if(!m_sdInvOp.hasType(DBCOM_ITEMRECORD(selectedItem.item.itemID).type)){
        return;
    }

    m_processRun->sendNPCEvent(m_sdInvOp.uid, {}, m_sdInvOp.commitTag.c_str(), str_printf("%d:%d", to_d(selectedItem.item.itemID), to_d(selectedItem.item.seqID)).c_str());
}

void InventoryBoard::removeItem(uint32_t itemID, uint32_t seqID, size_t count)
{
    auto grabbedItem = m_processRun->getMyHero()->getInvPack().getGrabbedItem();
    if((itemID == grabbedItem.itemID) && (seqID == grabbedItem.seqID)){
        if(count < grabbedItem.count){
            grabbedItem.count -= count;
            m_processRun->getMyHero()->getInvPack().setGrabbedItem(grabbedItem);
        }
        else{
            m_processRun->getMyHero()->getInvPack().setGrabbedItem({});
        }
    }
    else{
        if(m_selectedIndex >= 0){
            const auto &selectedItem = m_processRun->getMyHero()->getInvPack().getPackBinList().at(m_selectedIndex);
            if(selectedItem.item.itemID == itemID && selectedItem.item.seqID == seqID){
                m_selectedIndex = -1;
            }
        }
        m_processRun->getMyHero()->getInvPack().remove(itemID, seqID, count);
    }
}
