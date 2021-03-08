/*
 * =====================================================================================
 *
 *       Filename: purchaseboard.cpp
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

#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "purchaseboard.hpp"
#include "inputstringboard.hpp"

extern Client *g_client;
extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

PurchaseBoard::PurchaseBoard(ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          DIR_UPLEFT,
          0,
          0,
          0,
          0,

          widgetPtr,
          autoDelete
      }

    , m_closeButton
      {
          DIR_UPLEFT,
          257,
          183,
          {SYS_TEXNIL, 0X0000001C, 0X0000001D},

          nullptr,
          nullptr,
          [this]()
          {
              show(false);
              setExtendedItemID(0);
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

    , m_selectButton
      {
          DIR_UPLEFT,
          105,
          185,
          {SYS_TEXNIL, 0X08000003, 0X08000004},

          nullptr,
          nullptr,
          [this]()
          {
              setExtendedItemID(selectedItemID());
              m_closeExt1Button.setOff();
              m_closeExt2Button.setOff();
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

    , m_closeExt1Button
      {
          DIR_UPLEFT,
          448,
          159,
          {SYS_TEXNIL, 0X0000001C, 0X0000001D},

          nullptr,
          nullptr,
          [this]()
          {
              setExtendedItemID(0);
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

    , m_leftExt1Button
      {
          DIR_UPLEFT,
          315,
          163,
          {SYS_TEXNIL, 0X08000007, 0X08000008},

          nullptr,
          nullptr,
          [this]()
          {
              m_ext1Page = std::max<int>(0, m_ext1Page - 1);
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

    , m_selectExt1Button
      {
          DIR_UPLEFT,
          357,
          163,
          {SYS_TEXNIL, 0X08000005, 0X08000006},

          nullptr,
          nullptr,
          [this]()
          {
              const auto [itemID, seqID] = getExtSelectedItemSeqID();
              if(itemID){
                  m_processRun->requestBuy(m_npcUID, itemID, seqID, 1);
              }
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

    , m_rightExt1Button
      {
          DIR_UPLEFT,
          405,
          163,
          {SYS_TEXNIL, 0X08000009, 0X0800000A},

          nullptr,
          nullptr,
          [this]()
          {
              if(const auto ext1PageCount = extendedPageCount(); ext1PageCount > 0){
                  m_ext1Page = std::min<int>(ext1PageCount - 1, m_ext1Page + 1);
              }
              else{
                  m_ext1Page = 0;
              }
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

    , m_closeExt2Button
      {
          DIR_UPLEFT,
          474,
          56,
          {SYS_TEXNIL, 0X0000001C, 0X0000001D},

          nullptr,
          nullptr,
          [this]()
          {
              setExtendedItemID(0);
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

    , m_selectExt2Button
      {
          DIR_UPLEFT,
          366,
          60,
          {SYS_TEXNIL, 0X0800000B, 0X0800000C},

          nullptr,
          nullptr,
          [this]()
          {
              const auto [itemID, seqID] = getExtSelectedItemSeqID();
              if(!itemID){
                  return;
              }

              if(seqID != 0){
                  throw fflerror("unexpected seqID = %llu", to_llu(seqID));
              }

              const auto headerString = str_printf(u8"<par>请输入你要购买<t color=\"0xffff00ff\">%s</t>的数量</par>", to_cstr(DBCOM_ITEMRECORD(itemID).name));
              dynamic_cast<InputStringBoard *>(m_processRun->getWidget("InputStringBoard"))->waitInput(headerString, [itemID, npcUID = m_npcUID, this](std::u8string inputString)
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

          0,
          0,
          0,
          0,

          true,
          true,
          this,
          false,
      }

    , m_slider
      {
          DIR_UPLEFT,
          266,
          27,

          123,
          2,

          nullptr,
          this,
      }
    , m_processRun(runPtr)
{
    show(false);
    if(auto texPtr = g_progUseDB->Retrieve(0X08000000)){
        std::tie(m_w, m_h) = SDLDeviceHelper::getTextureSize(texPtr);
    }
    else{
        throw fflerror("no valid purchase status board frame texture");
    }
}

void PurchaseBoard::update(double)
{
}

void PurchaseBoard::drawEx(int dstX, int dstY, int, int, int, int) const
{
    if(auto pTexture = g_progUseDB->Retrieve(0X08000000 + extendedBoardGfxID())){
        g_sdlDevice->drawTexture(pTexture, dstX, dstY);
    }

    m_closeButton .draw();
    m_selectButton.draw();
    m_slider      .draw();

    int startY = m_startY;
    LabelBoard label
    {
        DIR_UPLEFT,
        0, // reset when drawing item
        0,
        u8"",

        1,
        12,
        0,

        colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
    };

    for(size_t startIndex = getStartIndex(), i = startIndex; i < std::min<size_t>(m_itemList.size(), startIndex + 4); ++i){
        if(const auto &ir = DBCOM_ITEMRECORD(m_itemList[i])){
            if(auto texPtr = g_itemDB->Retrieve(ir.pkgGfxID | 0X02000000)){
                const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
                const int drawX = m_startX + (m_boxW - texW) / 2;
                const int drawY =   startY + (m_boxH - texH) / 2;
                g_sdlDevice->drawTexture(texPtr, x() + drawX, y() + drawY);
            }

            label.setText(u8"%s", to_cstr(ir.name));
            label.moveTo(x() + m_startX + m_boxW + 10, y() + startY + (m_boxH - label.h()) / 2);
            label.draw();

            if(m_selected == to_d(i)){
                g_sdlDevice->fillRectangle(colorf::WHITE + 64, m_startX, startY, 252 - 19, m_boxH);
            }
            startY += m_lineH;
        }
    }

    switch(extendedBoardGfxID()){
        case 1: drawExt1(); break;
        case 2: drawExt2(); break;
        default: break;
    }
}

bool PurchaseBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return focusConsume(this, false);
    }

    if(!show()){
        return focusConsume(this, false);
    }

    if(m_closeButton.processEvent(event, valid)){
        return true;
    }

    if(m_selectButton.processEvent(event, valid)){
        return true;
    }

    if(m_slider.processEvent(event, valid)){
        return true;
    }

    switch(extendedBoardGfxID()){
        case 1:
            {
                if(m_closeExt1Button.processEvent(event, valid)){
                    return true;
                }

                if(m_leftExt1Button.processEvent(event, valid)){
                    return true;
                }

                if(m_selectExt1Button.processEvent(event, valid)){
                    return true;
                }

                if(m_rightExt1Button.processEvent(event, valid)){
                    return true;
                }
                break;
            }
        case 2:
            {
                if(m_closeExt2Button.processEvent(event, valid)){
                    return true;
                }

                if(m_selectExt2Button.processEvent(event, valid)){
                    if(m_processRun->getWidget("InputStringBoard")->focus()){
                        focus(false);
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

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                for(int i = 0; i < 4; ++i){
                    if(mathf::pointInRectangle<int>(event.button.x - x(), event.button.y - y(), 19, 15 + (57 - 15) * i, 252 - 19, 53 - 15)){
                        m_selected = i + getStartIndex();
                        if(event.button.clicks >= 2){
                            if(const auto itemID = selectedItemID()){
                                setExtendedItemID(itemID);
                            }
                        }
                        break;
                    }
                }

                if(const int gridIndex = getExt1PageGrid(); gridIndex >= 0 && extendedPageCount() > 0 && m_ext1Page * 3 * 4 + gridIndex < to_d(m_sdSellItemList.list.size())){
                    m_ext1PageGridSelected = m_ext1Page * 3 * 4 + gridIndex;
                }
                break;
            }
        case SDL_MOUSEWHEEL:
            {
                const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
                if(mathf::pointInRectangle<int>(mousePX - x(), mousePY - y(), 19, 15, 252 - 19, 15 + (57 - 15) * 4)){
                    if(m_itemList.size() > 4){
                        m_slider.addValue((event.wheel.y > 0 ? -1.0 : 1.0) / (m_itemList.size() - 4));
                    }
                }
                else if(extendedBoardGfxID() == 1 && extendedPageCount() > 0 && mathf::pointInRectangle<int>(mousePX - x(), mousePY - y(), 313, 41, 152, 114)){
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
        g_client->send(CM_QUERYSELLITEMLIST, cmQSIL);
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

int PurchaseBoard::getExt1PageGrid() const
{
    if(extendedBoardGfxID() != 1){
        return -1;
    }

    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
    if(mathf::pointInRectangle<int>(mousePX - x(), mousePY - y(), 313, 41, 152, 114)){
        const int r = (mousePY - y() -  41) / m_boxH;
        const int c = (mousePX - x() - 313) / m_boxW;
        return r * 4 + c;
    }
    return -1;
}

std::tuple<int, int, int, int> PurchaseBoard::getExt1PageGridLoc(int gridX, int gridY)
{
    if(gridX >= 0 && gridX < 4 && gridY >= 0 && gridY < 3){
        return
        {
            313 + m_boxW * gridX,
            41  + m_boxH * gridY,
            m_boxW,
            m_boxH,
        };
    }
    throw fflerror("invalid grid location: (%d, %d)", gridX, gridY);
}

void PurchaseBoard::drawExt1GridHoverText(int itemIndex) const
{
    if(extendedPageCount() <= 0){
        throw bad_reach();
    }

    if(!(itemIndex >= 0 && itemIndex < to_d(m_sdSellItemList.list.size()))){
        throw fflerror("invalid argument: itemIndex = %d", itemIndex);
    }

    const auto ir = DBCOM_ITEMRECORD(selectedItemID());
    const auto hoverText = str_printf
    (
        u8R"###( <layout>                                               )###""\n"
        u8R"###(     <par>【名称】%s</par>                              )###""\n"
        u8R"###(     <par>【售价】<t color='red+255'>%llu</t>金币</par> )###""\n"
        u8R"###(     <par></par>                                        )###""\n"
        u8R"###(     <par>%s</par>                                      )###""\n"
        u8R"###( </layout>                                              )###""\n",

        ir.name,
        to_llu(getItemPrice(itemIndex)),
        str_haschar(ir.description) ? ir.description : u8"暂无描述"
    );

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
        colorf::WHITE + 255,
        0,

        LALIGN_JUSTIFY,
    };

    hoverTextBoard.loadXML(to_cstr(hoverText));
    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
    g_sdlDevice->fillRectangle(colorf::RGBA(0, 0, 0, 200), mousePX, mousePY, std::max<int>(hoverTextBoard.w(), 200) + 20, hoverTextBoard.h() + 20);
    hoverTextBoard.drawAt(DIR_UPLEFT, mousePX + 10, mousePY + 10);
}

void PurchaseBoard::drawExt1() const
{
    if(extendedBoardGfxID() != 1){
        throw bad_reach();
    }

    m_closeExt1Button .draw();
    m_leftExt1Button  .draw();
    m_selectExt1Button.draw();
    m_rightExt1Button .draw();

    const int ext1PageCount = extendedPageCount();
    if(ext1PageCount <= 0){
        return;
    }

    if(m_ext1Page >= ext1PageCount){
        throw fflerror("invalid ext1Page: ext1Page = %d, listSize = %zu", m_ext1Page, m_sdSellItemList.list.size());
    }

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

            constexpr int rightStartX = 313;
            constexpr int rightStartY =  41;
            const int rightBoxX = rightStartX + c * m_boxW;
            const int rightBoxY = rightStartY + r * m_boxH;

            if(auto texPtr = g_itemDB->Retrieve(ir.pkgGfxID | 0X02000000)){
                const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
                const int rightDrawX = rightBoxX + (m_boxW - texW) / 2;
                const int rightDrawY = rightBoxY + (m_boxH - texH) / 2;
                g_sdlDevice->drawTexture(texPtr, x() + rightDrawX, y() + rightDrawY);
            }

            LabelBoard
            {
                DIR_UPLEFT,
                0,
                0,
                to_u8cstr(str_ksep(getItemPrice(i))),

                1,
                10,
                0,

                colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
            }.drawAt(DIR_UPLEFT, x() + rightBoxX, y() + rightBoxY);

            const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
            const bool gridSelected = (m_ext1PageGridSelected >= 0) && ((size_t)(m_ext1PageGridSelected) == i);
            const bool cursorOn = [rightBoxX, rightBoxY, mousePX, mousePY, this]() -> bool
            {
                return mathf::pointInRectangle<int>(mousePX, mousePY, x() + rightBoxX, y() + rightBoxY, m_boxW, m_boxH);
            }();

            if(gridSelected || cursorOn){
                const uint32_t gridColor = gridSelected ? (colorf::BLUE + 96) : (colorf::WHITE + 96);
                g_sdlDevice->fillRectangle(gridColor, x() + rightBoxX, y() + rightBoxY, m_boxW, m_boxH);
            }

            if(cursorOn){
                cursorOnGridIndex = to_d(i);
            }
        }
    }

    LabelBoard
    {
        DIR_UPLEFT,
        0,
        0,
        str_printf(u8"第%d/%d页", m_ext1Page + 1, ext1PageCount).c_str(),

        1,
        12,
        0,

        colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
    }.drawAt(DIR_NONE, x() + 389, y() + 18);

    if(cursorOnGridIndex >= 0){
        drawExt1GridHoverText(cursorOnGridIndex);
    }
}

void PurchaseBoard::drawExt2() const
{
    if(extendedBoardGfxID() != 2){
        throw bad_reach();
    }

    const auto [extItemID, extSeqID] = getExtSelectedItemSeqID();
    if(extSeqID){
        throw fflerror("unexpected extSeqID: %llu", to_llu(extSeqID));
    }

    m_closeExt2Button .draw();
    m_selectExt2Button.draw();

    if(m_sdSellItemList.list.empty()){
        return;
    }

    const auto &sellItem = m_sdSellItemList.list.at(0);
    const auto &ir = DBCOM_ITEMRECORD(sellItem.item.itemID);
    if(!ir){
        throw fflerror("bad item in sell list: itemID = %llu, seqID = %llu", to_llu(sellItem.item.itemID), to_llu(sellItem.item.seqID));
    }

    if(auto texPtr = g_itemDB->Retrieve(ir.pkgGfxID | 0X02000000)){
        constexpr int rightStartX = 303;
        constexpr int rightStartY =  16;
        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
        const int rightDrawX = rightStartX + (m_boxW - texW) / 2;
        const int rightDrawY = rightStartY + (m_boxH - texH) / 2;
        g_sdlDevice->drawTexture(texPtr, x() + rightDrawX, y() + rightDrawY);
    }

    LabelBoard
    {
        DIR_UPLEFT,
        0,
        0,
        to_u8cstr(str_ksep(getItemPrice(0)) + to_cstr(u8" 金币")),

        1,
        13,
        0,

        colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
    }.drawAt(DIR_LEFT, x() + 350 + 3, y() + 35);
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
                throw bad_reach();
            }
    }
}

size_t PurchaseBoard::getItemPrice(int itemIndex) const
{
    if(!(itemIndex >= 0 && itemIndex < to_d(m_sdSellItemList.list.size()))){
        throw fflerror("invalid argument: itemIndex = %d", itemIndex);
    }

    for(const auto &costItem: m_sdSellItemList.list.at(itemIndex).costList){
        if(costItem.itemID == DBCOM_ITEMID(u8"金币")){
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
