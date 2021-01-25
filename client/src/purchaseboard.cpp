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
#include "purchasecountboard.hpp"

extern Client *g_client;
extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

PurchaseBoard::PurchaseBoard(ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          0,
          0,
          0,
          0,

          widgetPtr,
          autoDelete
      }

    , m_closeButton
      {
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
          357,
          163,
          {SYS_TEXNIL, 0X08000005, 0X08000006},

          nullptr,
          nullptr,
          [this]()
          {
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
          366,
          60,
          {SYS_TEXNIL, 0X0800000B, 0X0800000C},

          nullptr,
          nullptr,
          [this]()
          {
              if(auto purchaseCountBoardPtr = dynamic_cast<PurchaseCountBoard *>(m_processRun->getWidget("PurchaseCountBoard"))){
                  purchaseCountBoardPtr->clear();
                  purchaseCountBoardPtr->show (true);
                  purchaseCountBoardPtr->focus(true);
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

    , m_slider
      {
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
        std::tie(m_w, m_h) = SDLDevice::getTextureSize(texPtr);
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

    // NPC sell items in the small box
    // +----------------------------
    // | (19, 15)                (252, 15)
    // |  *-----+----------------*
    // |  |     |                |
    // |  |     |(57, 53)        |
    // |  +-----*----------------+
    // | (19, 57)
    // |  *-----+-----------
    // |  |     |
    // |  |     |
    // |  +-----+-----------
    // |
    // +--------------------

    constexpr int startX = 19;
    /*     */ int startY = 15;

    constexpr int boxW  = 57 - 19;
    constexpr int boxH  = 53 - 15;
    constexpr int lineH = 57 - 15;

    LabelBoard label
    {
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
                const auto [texW, texH] = SDLDevice::getTextureSize(texPtr);
                const int drawX = startX + (boxW - texW) / 2;
                const int drawY = startY + (boxH - texH) / 2;
                g_sdlDevice->drawTexture(texPtr, x() + drawX, y() + drawY);

                label.setText(u8"%s", to_cstr(ir.name));
                label.moveTo(x() + startX + boxW + 10, y() + startY + (boxH - label.h()) / 2);
                label.draw();

                if(m_selected == (int)(i)){
                    g_sdlDevice->fillRectangle(colorf::WHITE + 64, startX, startY, 252 - 19, boxH);
                }
                startY += lineH;
            }
        }
    }

    switch(extendedBoardGfxID()){
        case 1:
            {
                int cursorOnGridIndex = -1;
                const int ext1PageCount = extendedPageCount();
                if(ext1PageCount > 0 && m_extendedItemID == m_sellItem.itemID){
                    if(const auto &ir = DBCOM_ITEMRECORD(m_extendedItemID)){
                        if(auto texPtr = g_itemDB->Retrieve(ir.pkgGfxID | 0X02000000)){
                            constexpr int rightStartX = 313;
                            constexpr int rightStartY =  41;
                            const auto [texW, texH] = SDLDevice::getTextureSize(texPtr);
                            if(m_ext1Page >= ext1PageCount){
                                throw fflerror("invalid ext1Page: ext1Page = %d, listSize = %zu", m_ext1Page, m_sellItem.list.data.size());
                            }

                            for(int r = 0; r < 3; ++r){
                                for(int c = 0; c < 4; ++c){
                                    const size_t i = m_ext1Page * 3 * 4 + r * 4 + c;
                                    if(i >= m_sellItem.list.data.size()){
                                        break;
                                    }

                                    const int rightBoxX = rightStartX + c * boxW;
                                    const int rightBoxY = rightStartY + r * boxH;
                                    const int rightDrawX = rightBoxX + (boxW - texW) / 2;
                                    const int rightDrawY = rightBoxY + (boxH - texH) / 2;
                                    const LabelBoard itemPrice
                                    {
                                        0, // reset by new width
                                        0,
                                        to_u8cstr(std::to_string(m_sellItem.list.data[i].price)),

                                        1,
                                        10,
                                        0,

                                        colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
                                    };

                                    g_sdlDevice->drawTexture(texPtr, x() + rightDrawX, y() + rightDrawY);
                                    itemPrice.drawAt(DIR_UPLEFT, x() + rightBoxX, y() + rightBoxY);

                                    const auto [mousePX, mousePY] = g_sdlDevice->getMousePLoc();
                                    const bool gridSelected = (m_ext1PageGridSelected >= 0) && ((size_t)(m_ext1PageGridSelected) == i);
                                    const bool cursorOn = [rightBoxX, rightBoxY, boxW, boxH, mousePX, mousePY, this]() -> bool
                                    {
                                        return mathf::pointInRectangle<int>(mousePX, mousePY, x() + rightBoxX, y() + rightBoxY, boxW, boxH);
                                    }();

                                    if(gridSelected || cursorOn){
                                        const uint32_t gridColor = gridSelected ? (colorf::BLUE + 96) : (colorf::WHITE + 96);
                                        g_sdlDevice->fillRectangle(gridColor, x() + rightBoxX, y() + rightBoxY, boxW, boxH);
                                    }

                                    if(cursorOn){
                                        cursorOnGridIndex = (int)(i);
                                    }
                                }
                            }
                        }
                    }

                    const LabelBoard pageIndexLabel
                    {
                        0, // reset by new width
                        0,
                        str_printf(u8"第%d/%d页", m_ext1Page + 1, ext1PageCount).c_str(),

                        1,
                        12,
                        0,

                        colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
                    };
                    pageIndexLabel.drawAt(DIR_NONE, x() + 389, y() + 18);
                }

                m_closeExt1Button .draw();
                m_leftExt1Button  .draw();
                m_selectExt1Button.draw();
                m_rightExt1Button .draw();

                if(cursorOnGridIndex >= 0){
                    drawExt1GridHoverText(cursorOnGridIndex);
                }
                break;
            }
        case 2:
            {
                if(const auto &ir = DBCOM_ITEMRECORD(m_extendedItemID)){
                    if(auto texPtr = g_itemDB->Retrieve(ir.pkgGfxID | 0X02000000)){
                        constexpr int rightStartX = 303;
                        constexpr int rightStartY =  16;
                        const auto [texW, texH] = SDLDevice::getTextureSize(texPtr);
                        const int rightDrawX = rightStartX + (boxW - texW) / 2;
                        const int rightDrawY = rightStartY + (boxH - texH) / 2;
                        g_sdlDevice->drawTexture(texPtr, x() + rightDrawX, y() + rightDrawY);
                        if(m_extendedItemID == m_sellItem.itemID){
                            const LabelBoard itemPrice
                            {
                                0, // reset by new width
                                0,
                                str_printf(u8"%s 金币", to_cstr(str_ksep(m_sellItem.single.price))).c_str(),

                                1,
                                13,
                                0,

                                colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
                            };
                            itemPrice.drawAt(DIR_LEFT, x() + 350 + 3, y() + 35);
                        }
                    }
                }

                m_closeExt2Button .draw();
                m_selectExt2Button.draw();
                break;
            }
        default:
            {
                break;
            }
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
                    if(m_processRun->getWidget("PurchaseCountBoard")->focus()){
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

                if(const int gridIndex = getExt1PageGrid(); gridIndex >= 0 && extendedPageCount() > 0 && m_ext1Page * 3 * 4 + gridIndex < (int)(m_sellItem.list.data.size())){
                    m_ext1PageGridSelected = m_ext1Page * 3 * 4 + gridIndex;
                }
                break;
            }
        case SDL_MOUSEWHEEL:
            {
                const auto [mousePX, mousePY] = g_sdlDevice->getMousePLoc();
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
    if((m_selected >= 0) && (m_selected < (int)(m_itemList.size()))){
        return m_itemList.at(m_selected);
    }
    return 0;
}

void PurchaseBoard::setExtendedItemID(uint32_t itemID)
{
    m_extendedItemID = itemID;
    m_ext1PageGridSelected = -1;

    if(m_extendedItemID){
        CMQuerySellItem cmQSI;
        std::memset(&cmQSI, 0, sizeof(cmQSI));
        cmQSI.npcUID = m_npcUID;
        cmQSI.itemID = m_extendedItemID;
        g_client->send(CM_QUERYSELLITEM, cmQSI);
    }
    else{
        m_sellItem.clear();
    }
}

void PurchaseBoard::setSellItem(SDSellItem sdSI)
{
    if(m_extendedItemID == sdSI.itemID){
        m_sellItem = std::move(sdSI);
    }
    else{
        m_sellItem.clear();
    }
    m_ext1Page = 0;
}

int PurchaseBoard::extendedBoardGfxID() const
{
    if(m_extendedItemID){
        return DBCOM_ITEMRECORD(m_extendedItemID).hasDBID() ? 1 : 2;
    }
    return 0;
}

int PurchaseBoard::extendedPageCount() const
{
    if(extendedBoardGfxID() != 1){
        return -1;
    }

    if(m_extendedItemID != m_sellItem.itemID){
        return 0;
    }

    if(m_sellItem.list.data.empty()){
        return 0;
    }
    return (int)(m_sellItem.list.data.size() + 11) / 12;
}

int PurchaseBoard::getExt1PageGrid() const
{
    if(extendedBoardGfxID() != 1){
        return -1;
    }

    const auto [mousePX, mousePY] = g_sdlDevice->getMousePLoc();
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

    if(!(itemIndex >= 0 && itemIndex < (int)(m_sellItem.list.data.size()))){
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
        to_llu(m_sellItem.list.data.at(itemIndex).price),
        str_nonempty(ir.description) ? ir.description : u8"暂无描述"
    );

    LayoutBoard hoverTextBoard
    {
        0,
        0,
        200,

        false,
        {0, 0, 0, 0},

        false,
        1,
        12,
    };

    hoverTextBoard.loadXML(to_cstr(hoverText));
    const auto [mousePX, mousePY] = g_sdlDevice->getMousePLoc();
    g_sdlDevice->fillRectangle(colorf::RGBA(0, 0, 0, 200), mousePX, mousePY, std::max<int>(hoverTextBoard.w(), 200) + 20, hoverTextBoard.h() + 20);
    hoverTextBoard.drawAt(DIR_UPLEFT, mousePX + 10, mousePY + 10);
}
