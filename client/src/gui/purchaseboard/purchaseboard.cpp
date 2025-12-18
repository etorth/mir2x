#include "totype.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "textboard.hpp"
#include "purchaseboard.hpp"
#include "inputstringboard.hpp"

extern Client *g_client;
extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

PurchaseBoard::PurchaseBoard(ProcessRun *argProc, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)
    , m_bg
      {{
          .texLoadFunc = [this]{ return g_progUseDB->retrieve(0X08000000 + extendedBoardGfxID()); },
          .parent{this},
      }}

    , m_buttonClose
      {{
          .x = 257,
          .y = 183,

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](Widget *, int)
          {
              setShow(false);
              setExtendedItemID(0);
          },

          .parent{this},
      }}

    , m_buttonSelect
      {{
          .x = 105,
          .y = 185,

          .texIDList
          {
              .on   = 0X08000003,
              .down = 0X08000004,
          },

          .onTrigger = [this](Widget *, int)
          {
              setExtendedItemID(selectedItemID());
              m_buttonExt1Close.setOff();
              m_buttonExt2Close.setOff();
          },

          .parent{this},
      }}

    , m_buttonExt1Close
      {{
          .x = 448,
          .y = 159,

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](Widget *, int)
          {
              setExtendedItemID(0);
          },

          .parent{this},
      }}

    , m_buttonExt1Left
      {{
          .x = 315,
          .y = 163,

          .texIDList
          {
              .on   = 0X08000007,
              .down = 0X08000008,
          },

          .onTrigger = [this](Widget *, int)
          {
              m_ext1Page = std::max<int>(0, m_ext1Page - 1);
          },

          .parent{this},
      }}

    , m_buttonExt1Select
      {{
          .x = 357,
          .y = 163,

          .texIDList
          {
              .on   = 0X08000005,
              .down = 0X08000006,
          },

          .onTrigger = [this](Widget *, int)
          {
              const auto [itemID, seqID] = getExtSelectedItemSeqID();
              if(itemID){
                  m_processRun->requestBuy(m_npcUID, itemID, seqID, 1);
              }
          },

          .parent{this},
      }}

    , m_buttonExt1Right
      {{
          .x = 405,
          .y = 163,

          .texIDList
          {
              .on   = 0X08000009,
              .down = 0X0800000A,
          },

          .onTrigger = [this](Widget *, int)
          {
              if(const auto ext1PageCount = extendedPageCount(); ext1PageCount > 0){
                  m_ext1Page = std::min<int>(ext1PageCount - 1, m_ext1Page + 1);
              }
              else{
                  m_ext1Page = 0;
              }
          },

          .parent{this},
      }}

    , m_buttonExt2Close
      {{
          .x = 474,
          .y = 56,

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](Widget *, int)
          {
              setExtendedItemID(0);
          },

          .parent{this},
      }}

    , m_buttonExt2Select
      {{
          .x = 366,
          .y = 60,

          .texIDList
          {
              .on   = 0X0800000B,
              .down = 0X0800000C,
          },

          .onTrigger = [this](Widget *, int)
          {
              const auto [itemID, seqID] = getExtSelectedItemSeqID();
              if(!itemID){
                  return;
              }

              if(seqID != 0){
                  throw fflerror("unexpected seqID = %llu", to_llu(seqID));
              }

              auto inputBoardPtr = dynamic_cast<InputStringBoard *>(m_processRun->getWidget("InputStringBoard"));
              const auto headerString = str_printf(u8"<layout><par>请输入你要购买<t color=\"red\">%s</t>的数量</par></layout>", to_cstr(DBCOM_ITEMRECORD(itemID).name));

              inputBoardPtr->setSecurity(false);
              inputBoardPtr->waitInput(headerString, [itemID, npcUID = m_npcUID, this](std::u8string inputString)
              {
                  const auto &ir = DBCOM_ITEMRECORD(itemID);
                  int count = 0;
                  try{
                      count = std::stoi(to_cstr(inputString));
                  }
                  catch(...){
                      m_processRun->addCBLog(CBLOG_ERR, u8"无效输入:%s", to_cstr(inputString));
                  }

                  if(ir && count > 0){
                      m_processRun->requestBuy(npcUID, itemID, 0, count);
                  }
              });
          },

          .parent{this},
      }}

    , m_slider
      {{
          .bar
          {
              .x = 266,
              .y = 25,
              .w = 7,
              .h = 125,
          },

          .index = 0,
          .parent{this},
      }}
{
    setShow(false);
    setSize([this]{ return m_bg.w(); },
            [this]{ return m_bg.h(); });
}

void PurchaseBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    drawChild(&m_bg          , m);
    drawChild(&m_buttonClose , m);
    drawChild(&m_buttonSelect, m);
    drawChild(&m_slider      , m);

    int startY = m_startY;
    const char * itemName = nullptr;

    TextBoard label
    {{
        .textFunc = [&itemName]{ return to_cstr(itemName); },
        .font
        {
            .id = 1,
            .size = 12,
            .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
        },
    }};

    for(size_t startIndex = getStartIndex(), i = startIndex; i < std::min<size_t>(m_itemList.size(), startIndex + 4); ++i){
        if(const auto &ir = DBCOM_ITEMRECORD(m_itemList[i])){
            drawItemInGrid(ir.type, ir.pkgGfxID, m_startX, startY, m);

            itemName = to_cstr(ir.name);
            drawAsChild(&label, DIR_LEFT, m_startX + m_boxW + 10, startY + m_boxH / 2, m);

            if(m_selected == to_d(i)){
                GfxShapeBoard mask
                {{
                    .w = 252 - 19,
                    .h = m_boxH,

                    .drawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
                    {
                        g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(64), dstDrawX, dstDrawY, self->w(), self->h());
                    },
                }};
                drawAsChild(&mask, DIR_UPLEFT, m_startX, startY, m);
            }
            startY += m_lineH;
        }
    }

    switch(extendedBoardGfxID()){
        case 1: drawExt1(m); break;
        case 2: drawExt2(m); break;
        default: break;
    }
}

bool PurchaseBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_buttonClose .processEventParent(event, valid, m)){ return true; }
    if(m_buttonSelect.processEventParent(event, valid, m)){ return true; }
    if(m_slider      .processEventParent(event, valid, m)){ return true; }

    switch(extendedBoardGfxID()){
        case 1:
            {
                if(m_buttonExt1Close .processEventParent(event, valid, m)){ return true; }
                if(m_buttonExt1Left  .processEventParent(event, valid, m)){ return true; }
                if(m_buttonExt1Select.processEventParent(event, valid, m)){ return true; }
                if(m_buttonExt1Right .processEventParent(event, valid, m)){ return true; }
                break;
            }
        case 2:
            {
                if(m_buttonExt2Close.processEventParent(event, valid, m)){
                    return true;
                }

                if(m_buttonExt2Select.processEventParent(event, valid, m)){
                    if(m_processRun->getWidget("InputStringBoard")->focus()){
                        setFocus(false);
                    }
                    return false;
                }
                break;
            }
        default:
            {
                break;
            }
    }

    const auto remapX = m.x - m.ro->x;
    const auto remapY = m.y - m.ro->y;

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                const int mouseDX = event.button.x - remapX;
                const int mouseDY = event.button.y - remapY;

                for(int i = 0; i < 4; ++i){
                    const Widget::ROI select
                    {
                        .x = m_startX,
                        .y = m_startY + m_lineH * i,

                        .w = m_lineW,
                        .h = m_boxH,
                    };


                    if(select.in(mouseDX, mouseDY)){
                        m_selected = i + getStartIndex();
                        if(event.button.clicks >= 2){
                            if(const auto itemID = selectedItemID()){
                                setExtendedItemID(itemID);
                            }
                        }
                        break;
                    }
                }

                if(const int gridIndex = getExt1PageGrid(mouseDX, mouseDY); (gridIndex >= 0) && (extendedPageCount() > 0) && (m_ext1Page * 3 * 4 + gridIndex < to_d(m_sdSellItemList.list.size()))){
                    m_ext1PageGridSelected = m_ext1Page * 3 * 4 + gridIndex;
                }
                break;
            }
        case SDL_MOUSEWHEEL:
            {
                const Widget::ROI select
                {
                    .x = m_startX,
                    .y = m_startY,

                    .w = m_lineW,
                    .h = m_lineH * 3 + m_boxH,
                };

                const int mouseDX = event.wheel.mouseX - remapX;
                const int mouseDY = event.wheel.mouseY - remapY;

                if(select.in(mouseDX, mouseDY)){
                    if(m_itemList.size() > 4){
                        m_slider.addValue((event.wheel.y > 0 ? -1.0 : 1.0) / (m_itemList.size() - 4), false);
                    }
                }
                else if((extendedBoardGfxID() == 1) && (extendedPageCount() > 0) && m_ext1GridArea.in(mouseDX, mouseDY)){
                    if(event.wheel.y > 0){
                        m_ext1Page--;
                    }
                    else{
                        m_ext1Page++;
                    }
                    m_ext1Page = std::max<int>(0, std::min<int>(extendedPageCount() - 1, m_ext1Page));
                }
                break;
            }
        default:
            {
                break;
            }
    }
    return true;
}

void PurchaseBoard::loadSell(uint64_t npcUID, std::vector<uint32_t> itemList)
{
    m_npcUID = npcUID;
    m_itemList = std::move(itemList);
}

size_t PurchaseBoard::getStartIndex() const
{
    if(m_itemList.size() <= 4){
        return 0;
    }
    return std::lround((m_itemList.size() - 4) * m_slider.getValue());
}

uint32_t PurchaseBoard::selectedItemID() const
{
    if((m_selected >= 0) && (m_selected < to_d(m_itemList.size()))){
        return m_itemList.at(m_selected);
    }
    return 0;
}

void PurchaseBoard::setExtendedItemID(uint32_t itemID)
{
    m_ext1PageGridSelected = -1;
    if(itemID){
        CMQuerySellItemList cmQSIL;
        std::memset(&cmQSIL, 0, sizeof(cmQSIL));
        cmQSIL.npcUID = m_npcUID;
        cmQSIL.itemID =   itemID;
        g_client->send({CM_QUERYSELLITEMLIST, cmQSIL});
    }
    else{
        m_sdSellItemList.clear();
    }
}

void PurchaseBoard::setSellItemList(SDSellItemList sdSIL)
{
    if(m_npcUID == sdSIL.npcUID){
        m_sdSellItemList = std::move(sdSIL);
    }
    else{
        m_sdSellItemList.clear();
    }
    m_ext1Page = 0;
}

int PurchaseBoard::extendedBoardGfxID() const
{
    if(m_sdSellItemList.npcUID && !m_sdSellItemList.list.empty()){
        return DBCOM_ITEMRECORD(m_sdSellItemList.list.front().item.itemID).packable() ? 2 : 1;
    }
    return 0;
}

int PurchaseBoard::extendedPageCount() const
{
    if(extendedBoardGfxID() != 1){
        return -1;
    }

    if(m_sdSellItemList.list.empty()){
        return 0;
    }
    return to_d(m_sdSellItemList.list.size() + 11) / 12;
}

int PurchaseBoard::getExt1PageGrid(int mouseDX, int mouseDY) const
{
    if(extendedBoardGfxID() != 1){
        return -1;
    }

    if(m_ext1GridArea.in(mouseDX, mouseDY)){
        const int r = (mouseDY - m_ext1GridArea.y) / m_boxH;
        const int c = (mouseDX - m_ext1GridArea.x) / m_boxW;
        return r * 4 + c;
    }
    return -1;
}

std::tuple<int, int, int, int> PurchaseBoard::getExt1PageGridLoc(int gridX, int gridY)
{
    fflassert(gridX >= 0 && gridX < 4, gridX, gridY);
    fflassert(gridY >= 0 && gridY < 3, gridX, gridY);

    return
    {
        m_ext1GridArea.x + m_boxW * gridX,
        m_ext1GridArea.y + m_boxH * gridY,
        m_boxW,
        m_boxH,
    };
}

void PurchaseBoard::drawExt1GridHoverText(int itemIndex) const
{
    if(extendedPageCount() <= 0){
        throw fflreach();
    }

    fflassert(itemIndex >= 0);
    fflassert(itemIndex < to_d(m_sdSellItemList.list.size()));
    fflassert(DBCOM_ITEMRECORD(selectedItemID()));

    LayoutBoard hoverTextBoard
    {{
        .lineWidth = 200,
        .font
        {
            .id = 1,
            .size = 12,
        },

        .lineAlign = LALIGN_JUSTIFY,
    }};

    const auto goldPrice = getItemPrice(itemIndex);
    hoverTextBoard.loadXML(to_cstr(m_sdSellItemList.list.at(itemIndex).item.getXMLLayout(
    {
        {SDItem::XML_PRICE, std::to_string(goldPrice)},
        {SDItem::XML_PRICECOLOR, (goldPrice > 100) ? std::string("red") : std::string("green")},
    }).c_str()));

    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
    g_sdlDevice->fillRectangle(colorf::RGBA(0, 0, 0, 200), mousePX, mousePY, std::max<int>(hoverTextBoard.w(), 200) + 20, hoverTextBoard.h() + 20);
    hoverTextBoard.draw({.x=mousePX + 10, .y=mousePY + 10});
}

void PurchaseBoard::drawExt1(Widget::ROIMap m) const
{
    fflassert(extendedBoardGfxID() == 1);

    drawChild(&m_buttonExt1Close , m);
    drawChild(&m_buttonExt1Left  , m);
    drawChild(&m_buttonExt1Select, m);
    drawChild(&m_buttonExt1Right , m);

    const int ext1PageCount = extendedPageCount();
    if(ext1PageCount <= 0){
        return;
    }

    if(m_ext1Page >= ext1PageCount){
        throw fflerror("invalid ext1Page: ext1Page = %d, listSize = %zu", m_ext1Page, m_sdSellItemList.list.size());
    }

    const auto remapX = m.x - m.ro->x;
    const auto remapY = m.y - m.ro->y;

    int cursorOnGridIndex = -1;
    for(int r = 0; r < 3; ++r){
        for(int c = 0; c < 4; ++c){
            const size_t i = m_ext1Page * 3 * 4 + r * 4 + c;
            if(i >= m_sdSellItemList.list.size()){
                break;
            }

            const auto &sellItem = m_sdSellItemList.list.at(i);
            const auto &ir = DBCOM_ITEMRECORD(sellItem.item.itemID);
            if(!ir){
                throw fflerror("bad item in sell list: itemID = %llu, seqID = %llu", to_llu(sellItem.item.itemID), to_llu(sellItem.item.seqID));
            }

            constexpr int rightStartX = m_ext1GridArea.x;
            constexpr int rightStartY = m_ext1GridArea.y;

            const int rightBoxX = rightStartX + c * m_boxW;
            const int rightBoxY = rightStartY + r * m_boxH;

            drawItemInGrid(ir.type, ir.pkgGfxID, rightBoxX, rightBoxY, m);

            const TextBoard price
            {{
                .textFunc = str_ksep(getItemPrice(i)),
                .font
                {
                    .id = 1,
                    .size = 10,
                    .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
                },
            }};
            drawAsChild(&price, DIR_UPLEFT, rightBoxX, rightBoxY, m);

            const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
            const bool gridSelected = (m_ext1PageGridSelected >= 0) && ((size_t)(m_ext1PageGridSelected) == i);
            const bool cursorOn = [rightBoxX, rightBoxY, mousePX, mousePY, remapX, remapY, this]() -> bool
            {
                return mathf::pointInRectangle<int>(mousePX, mousePY, remapX + rightBoxX, remapY + rightBoxY, m_boxW, m_boxH);
            }();

            if(gridSelected || cursorOn){
                const uint32_t gridColor = gridSelected ? (colorf::BLUE + colorf::A_SHF(96)) : (colorf::WHITE + colorf::A_SHF(96));
                g_sdlDevice->fillRectangle(gridColor, remapX + rightBoxX, remapY + rightBoxY, m_boxW, m_boxH);
            }

            if(cursorOn){
                cursorOnGridIndex = to_d(i);
            }
        }
    }

    LabelBoard
    {{
        .label = str_printf(u8"第%d/%d页", m_ext1Page + 1, ext1PageCount).c_str(),
        .font
        {
            .id = 1,
            .size = 12,
            .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
        },
    }}.draw({.dir=DIR_NONE, .x=remapX + 389, .y=remapY + 18});

    if(cursorOnGridIndex >= 0){
        drawExt1GridHoverText(cursorOnGridIndex);
    }
}

void PurchaseBoard::drawExt2(Widget::ROIMap m) const
{
    fflassert(extendedBoardGfxID() == 2);

    const auto [extItemID, extSeqID] = getExtSelectedItemSeqID();
    if(extSeqID){
        throw fflerror("unexpected extSeqID: %llu", to_llu(extSeqID));
    }

    drawChild(&m_buttonExt2Close , m);
    drawChild(&m_buttonExt2Select, m);

    if(m_sdSellItemList.list.empty()){
        return;
    }

    const auto &sellItem = m_sdSellItemList.list.at(0);
    const auto &ir = DBCOM_ITEMRECORD(sellItem.item.itemID);
    if(!ir){
        throw fflerror("bad item in sell list: itemID = %llu, seqID = %llu", to_llu(sellItem.item.itemID), to_llu(sellItem.item.seqID));
    }

    drawItemInGrid(ir.type, ir.pkgGfxID, m_ext2GridArea.x, m_ext2GridArea.y, m);

    const TextBoard price
    {{
        .textFunc = str_ksep(getItemPrice(0)) + to_cstr(u8" 金币"),
        .font
        {
            .id = 1,
            .size = 13,
            .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
        },
    }};
    drawAsChild(&price, DIR_LEFT, m_ext2PriceArea.x + 3, m_ext2PriceArea.y + m_ext2PriceArea.h / 2, m);
}

std::tuple<uint32_t, uint32_t> PurchaseBoard::getExtSelectedItemSeqID() const
{
    switch(extendedBoardGfxID()){
        case 1:
            {
                if(m_ext1PageGridSelected >= 0 && m_ext1PageGridSelected < to_d(m_sdSellItemList.list.size())){
                    const auto &sellItem = m_sdSellItemList.list.at(m_ext1PageGridSelected);
                    return {sellItem.item.itemID, sellItem.item.seqID};
                }
                return {0, 0};
            }
        case 2:
            {
                if(m_sdSellItemList.list.empty()){
                    return {0, 0};
                }
                return {m_sdSellItemList.list.front().item.itemID, 0};
            }
        default:
            {
                throw fflreach();
            }
    }
}

size_t PurchaseBoard::getItemPrice(int itemIndex) const
{
    if(!(itemIndex >= 0 && itemIndex < to_d(m_sdSellItemList.list.size()))){
        throw fflerror("invalid argument: itemIndex = %d", itemIndex);
    }

    for(const auto &costItem: m_sdSellItemList.list.at(itemIndex).costList){
        if(costItem.isGold()){
            return costItem.count;
        }
    }
    return 0;
}

void PurchaseBoard::onBuySucceed(uint64_t npcUID, uint32_t itemID, uint32_t seqID)
{
    if(npcUID != m_npcUID){
        return;
    }

    if(npcUID != m_sdSellItemList.npcUID){
        return;
    }

    if(DBCOM_ITEMRECORD(itemID).packable()){
        return;
    }

    for(auto p = m_sdSellItemList.list.begin(); p != m_sdSellItemList.list.end(); ++p){
        if(p->item.itemID == itemID && p->item.seqID == seqID){
            m_sdSellItemList.list.erase(p);
            return;
        }
    }
}

void PurchaseBoard::drawItemInGrid(const char8_t *itemType, uint32_t pkgGfxID, int gridDX, int gridDY, Widget::ROIMap pm) const
{
    if(auto texPtr = getItemTexture(itemType, pkgGfxID)){
        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
        const auto resizeRatio = std::max<double>({to_df(texW) / m_boxW, to_df(texH) / m_boxH, 1.0});

        const auto drawW = to_d(std::lround(texW / resizeRatio));
        const auto drawH = to_d(std::lround(texH / resizeRatio));

        const ImageBoard img
        {{
            .w = drawW,
            .h = drawH,
            .texLoadFunc = texPtr,
        }};

        img.drawRoot(pm.create({gridDX + (m_boxW - drawW) / 2, gridDY + (m_boxH - drawH) / 2, drawW, drawH}));
    }
}

SDL_Texture *PurchaseBoard::getItemTexture(const char8_t *itemType, uint32_t pkgGfxID)
{
    // for some items the game originally doesn't support to buy/sell them so there is no buy/sell gfx
    // but there is gfx when they get weared, try to reuse

    if(auto texPtr = g_itemDB->retrieve(pkgGfxID | 0X02000000)){
        return texPtr;
    }

    if(str_haschar(itemType)){
        if(false
                || to_u8sv(itemType) == u8"项链"
                || to_u8sv(itemType) == u8"戒指"
                || to_u8sv(itemType) == u8"手镯"
                || to_u8sv(itemType) == u8"鞋"
                || to_u8sv(itemType) == u8"道具"
                || to_u8sv(itemType) == u8"火把"
                || to_u8sv(itemType) == u8"头盔"
                || to_u8sv(itemType) == u8"武器"
                || to_u8sv(itemType) == u8"衣服"){
            return g_itemDB->retrieve(pkgGfxID | 0X01000000);
        }
    }
    return nullptr;
}
