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
#include "purchasecountboard.hpp"

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
              m_extended = 0;
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
              m_extended = 2;
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
              m_extended = 0;
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
              m_extended = 0;
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
              m_processRun->getWidget("PurchaseCountBoard")->show (true);
              m_processRun->getWidget("PurchaseCountBoard")->focus(true);
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
    if(auto pTexture = g_progUseDB->Retrieve(0X08000000 + m_extended)){
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
        this,
    };

    for(size_t startIndex = getStartIndex(), i = startIndex; i < std::min<size_t>(m_itemList.size(), startIndex + 4); ++i){
        if(const auto &ir = DBCOM_ITEMRECORD(m_itemList[i])){
            if(auto texPtr = g_itemDB->Retrieve(ir.pkgGfxID | 0X02000000)){
                const auto [texW, texH] = SDLDevice::getTextureSize(texPtr);
                const int drawX = startX + (boxW - texW) / 2;
                const int drawY = startY + (boxH - texH) / 2;
                g_sdlDevice->drawTexture(texPtr, x() + drawX, y() + drawY);

                label.setText(u8"%s", to_cstr(ir.name));
                label.moveTo(dx() + startX + boxW + 10, startY + (boxH - label.h()) / 2);
                label.draw();

                if(m_selected == (int)(i)){
                    g_sdlDevice->fillRectangle(colorf::WHITE + 64, startX, startY, 252 - 19, boxH);
                }
                startY += lineH;
            }
        }
    }

    switch(m_extended){
        case 1:
            {
                m_closeExt1Button .draw();
                m_leftExt1Button  .draw();
                m_selectExt1Button.draw();
                m_rightExt1Button .draw();
                break;
            }
        case 2:
            {
                m_closeExt2Button .draw();
                m_selectExt2Button.draw();
                break;
            }
        case 0:
            {
                break;
            }
        default:
            {
                throw fflerror("invalid extended argument: %d", m_extended);
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

    switch(m_extended){
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
        case 0:
            {
                break;
            }
        default:
            {
                throw fflerror("invalid extended argument: %d", m_extended);
            }
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                int selected = -1;
                for(int i = 0; i < 4; ++i){
                    if(mathf::pointInRectangle<int>(event.button.x - x(), event.button.y - y(), 19, 15 + (57 - 15) * i, 252 - 19, 53 - 15)){
                        selected = i;
                        break;
                    }
                }

                if(selected >= 0){
                    m_selected = selected + getStartIndex();
                }
                break;
            }
        case SDL_MOUSEWHEEL:
            {
                int mousePX = -1;
                int mousePY = -1;
                SDL_GetMouseState(&mousePX, &mousePY);
                if(mathf::pointInRectangle<int>(mousePX - x(), mousePY - y(), 19, 15, 252 - 19, 15 + (57 - 15) * 4)){
                    if(m_itemList.size() > 4){
                        m_slider.addValue((event.wheel.y > 0 ? -1.0 : 1.0) / (m_itemList.size() - 4));
                    }
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
