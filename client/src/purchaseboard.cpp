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

#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "purchaseboard.hpp"

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

    , m_slider
      {
          267,
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

void PurchaseBoard::drawEx(int dstX, int dstY, int, int, int, int)
{
    if(auto pTexture = g_progUseDB->Retrieve(0X08000000)){
        g_sdlDevice->drawTexture(pTexture, dstX, dstY);
    }

    m_closeButton .draw();
    m_selectButton.draw();
    m_slider      .draw();

    // NPC sell items in the small box
    // +--------------------
    // | (19, 15)
    // |  *-----+-----------
    // |  |     |
    // |  |     |(57, 53)
    // |  +-----*-----------
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

    const auto startIndex = [this]() -> size_t
    {
        if(m_itemList.size() <= 4){
            return 0;
        }
        return std::lround((m_itemList.size() - 4) * m_slider.getValue());
    }();

    for(size_t i = startIndex; i < std::min<size_t>(m_itemList.size(), startIndex + 4); ++i){
        if(const auto &ir = DBCOM_ITEMRECORD(m_itemList[i])){
            if(auto texPtr = g_itemDB->Retrieve(ir.pkgGfxID | 0X02000000)){
                const auto [texW, texH] = SDLDevice::getTextureSize(texPtr);
                const int drawX = startX + (boxW - texW) / 2;
                const int drawY = startY + (boxH - texH) / 2;
                g_sdlDevice->drawTexture(texPtr, x() + drawX, y() + drawY);
                startY += lineH;
            }
        }
    }
}

bool PurchaseBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return focusConsumer(this, false);
    }

    if(!show()){
        return focusConsumer(this, false);
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
    return false;
}

void PurchaseBoard::loadSell(uint64_t npcUID, std::vector<uint32_t> itemList)
{
    m_npcUID = npcUID;
    m_itemList = std::move(itemList);
}
