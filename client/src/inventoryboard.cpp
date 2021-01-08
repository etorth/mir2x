/*
 * =====================================================================================
 *
 *       Filename: inventoryboard.cpp
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
#include "inventoryboard.hpp"

extern PNGTexDB *g_progUseDB;
extern PNGTexDB *g_itemDB;
extern SDLDevice *g_sdlDevice;

InventoryBoard::InventoryBoard(int nX, int nY, ProcessRun *pRun, Widget *pwidget, bool autoDelete)
    : Widget(nX, nY, 0, 0, pwidget, autoDelete)
    , m_opNameBoard
      {
          132,
          16,
          u8"【背包】",

          1,
          12,
          0,

          colorf::WHITE + 255,
          this,
      }

    , m_wmdAniBoard
      {
          23,
          14,
          this,
      }

    , m_slider
      {
          258,
          64,

          291,
          2,

          nullptr,
          this,
      }

    , m_closeButton
      {
          242,
          422,
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
      }
    , m_processRun(pRun)
{
    show(false);
    auto texPtr = g_progUseDB->Retrieve(0X0000001B);
    if(!texPtr){
        throw fflerror("no valid inventory frame texture");
    }
    std::tie(m_w, m_h) = SDLDevice::getTextureSize(texPtr);
}

void InventoryBoard::drawItem(int dstX, int dstY, size_t startRow, const PackBin &bin) const
{
    if(true
            && bin
            && bin.X >= 0
            && bin.Y >= 0
            && bin.W >  0
            && bin.H >  0){

        if(auto texPtr = g_itemDB->Retrieve(DBCOM_ITEMRECORD(bin.ID).pkgGfxID | 0X01000000)){
            constexpr int invGridX0 = 18;
            constexpr int invGridY0 = 59;
            const auto [itemPW, itemPH] = SDLDevice::getTextureSize(texPtr);

            const int startX = dstX + invGridX0;
            const int startY = dstY + invGridY0 - startRow * SYS_INVGRIDPH;
            const int  viewX = dstX + invGridX0;
            const int  viewY = dstY + invGridY0;

            int drawDstX = startX + bin.X * SYS_INVGRIDPW + (bin.W * SYS_INVGRIDPW - itemPW) / 2;
            int drawDstY = startY + bin.Y * SYS_INVGRIDPH + (bin.H * SYS_INVGRIDPH - itemPH) / 2;
            int drawSrcX = 0;
            int drawSrcY = 0;
            int drawSrcW = itemPW;
            int drawSrcH = itemPH;

            if(mathf::ROICrop(
                        &drawSrcX, &drawSrcY,
                        &drawSrcW, &drawSrcH,
                        &drawDstX, &drawDstY,

                        drawSrcW,
                        drawSrcH,

                        0, 0, -1, -1,
                        viewX, viewY, SYS_INVGRIDPW * 6, SYS_INVGRIDPH * 8)){
                g_sdlDevice->drawTexture(texPtr, drawDstX, drawDstY, drawSrcX, drawSrcY, drawSrcW, drawSrcH);
            }
        }
    }
}

void InventoryBoard::update(double fUpdateTime)
{
    m_wmdAniBoard.update(fUpdateTime);
}

void InventoryBoard::drawEx(int dstX, int dstY, int, int, int, int) const
{
    if(auto pTexture = g_progUseDB->Retrieve(0X0000001B)){
        g_sdlDevice->drawTexture(pTexture, dstX, dstY);
    }

    if(auto myHeroPtr = m_processRun->getMyHero()){
        const size_t startRow = getStartRow();
        for(auto &bin: myHeroPtr->getInvPack().GetPackBinList()){
            drawItem(dstX, dstY, startRow, bin);
        }
    }

    drawGold();
    m_opNameBoard.draw();
    m_wmdAniBoard.draw();
    m_slider     .draw();
    m_closeButton.draw();
}

bool InventoryBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        focus(false);
        return false;
    }

    if(!show()){
        return false;
    }

    if(m_closeButton.processEvent(event, valid)){
        return true;
    }

    if(m_slider.processEvent(event, valid)){
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
                            return focusConsume(this, in(event.button.x, event.button.y));
                        }
                    default:
                        {
                            return focusConsume(this, false);
                        }
                }
            }
        case SDL_MOUSEWHEEL:
            {
                constexpr int invGridX0 = 18;
                constexpr int invGridY0 = 59;
                const auto [mousePX, mousePY] = g_sdlDevice->getMousePLoc();

                if(mathf::pointInRectangle<int>(mousePX, mousePY, x() + invGridX0, y() + invGridY0, 6 * SYS_INVGRIDPW, 8 * SYS_INVGRIDPH)){
                    const auto rowCount = getRowCount();
                    if(rowCount > 8){
                        m_slider.addValue((event.wheel.y > 0 ? -1.0 : 1.0) / (rowCount - 8));
                    }
                    return focusConsume(this, true);
                }
                return false;
            }
        default:
            {
                return focusConsume(this, false);
            }
    }
}

std::string InventoryBoard::getGoldStr() const
{
    return str_ksep([this]() -> int
    {
        if(auto p = m_processRun->getMyHero()){
            return p->GetGold();
        }
        return 0;
    }(), ',');
}

size_t InventoryBoard::getRowCount() const
{
    const auto &packBinList = m_processRun->getMyHero()->getInvPack().GetPackBinList();
    if(packBinList.empty()){
        return 0;
    }

    size_t rowCount = 0;
    for(const auto &bin: packBinList){
        rowCount = std::max<size_t>(rowCount, bin.Y + bin.H);
    }
    return rowCount;
}

size_t InventoryBoard::getStartRow() const
{
    const size_t rowGfxCount = 8;
    const size_t rowCount = getRowCount();

    if(rowCount <= rowGfxCount){
        return 0;
    }
    return std::lround((rowCount - rowGfxCount) * m_slider.getValue());
}

void InventoryBoard::drawGold() const
{
    LabelBoard goldBoard
    {
        0, // reset by new width
        0,
        to_u8cstr(getGoldStr()),

        1,
        12,
        0,

        colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
    };

    goldBoard.moveBy(x() + 105 - goldBoard.w() / 2, y() + 401);
    goldBoard.draw();
}
