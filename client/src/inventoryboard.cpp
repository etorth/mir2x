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

void InventoryBoard::drawItem(int dstX, int dstY, size_t startRow, bool selected, const PackBin &bin) const
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

            int binGridX = bin.X;
            int binGridY = bin.Y;
            int binGridW = bin.W;
            int binGridH = bin.H;

            if(selected && mathf::rectangleOverlapRegion<int>(0, startRow, 6, 8, &binGridX, &binGridY, &binGridW, &binGridH)){
                g_sdlDevice->fillRectangle(colorf::WHITE + 128,
                        startX + binGridX * SYS_INVGRIDPW,
                        startY + binGridY * SYS_INVGRIDPH, // startY is for (0, 0), not for (0, startRow)
                        binGridW * SYS_INVGRIDPW,
                        binGridH * SYS_INVGRIDPH);
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
        const auto startRow = getStartRow();
        const auto &packBinListCRef = myHeroPtr->getInvPack().GetPackBinList();
        for(int i = 0; i < (int)(packBinListCRef.size()); ++i){
            drawItem(dstX, dstY, startRow, (i == m_selectedPackBinIndex), packBinListCRef.at(i));
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
                            if(in(event.button.x, event.button.y)){
                                if(const int selectedPackIndex = getPackBinIndex(event.button.x, event.button.y); selectedPackIndex >= 0){
                                    m_selectedPackBinIndex = selectedPackIndex;
                                }
                                else{
                                    m_selectedPackBinIndex = -1;
                                }
                                return focusConsume(this, true);
                            }
                            return focusConsume(this, false);
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
    const LabelBoard goldBoard
    {
        0, // reset by new width
        0,
        to_u8cstr(getGoldStr()),

        1,
        12,
        0,

        colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
    };
    goldBoard.drawAt(DIR_NONE, x() + 106, y() + 409);
}

int InventoryBoard::getPackBinIndex(int locPX, int locPY) const
{
    const auto [gridX, gridY] = getInvGrid(locPX, locPY);
    if(gridX < 0 || gridY < 0){
        return -1;
    }

    const auto myHeroPtr = m_processRun->getMyHero();
    if(!myHeroPtr){
        return -1;
    }

    const auto startRow = getStartRow();
    const auto &packBinListCRef = myHeroPtr->getInvPack().GetPackBinList();
    for(int i = 0; i < (int)(packBinListCRef.size()); ++i){
        const auto &packBinCRef = packBinListCRef.at(i);
        if(mathf::pointInRectangle<int>(gridX, gridY, packBinCRef.X, packBinCRef.Y - startRow, packBinCRef.W, packBinCRef.H)){
            return i;
        }
    }
    return -1;
}

std::tuple<int, int> InventoryBoard::getInvGrid(int locPX, int locPY) const
{
    constexpr int invGridX0 = 18;
    constexpr int invGridY0 = 59;
    const     int gridPX0 = invGridX0 + x();
    const     int gridPY0 = invGridY0 + y();

    if(!mathf::pointInRectangle<int>(locPX, locPY, gridPX0, gridPY0, 6 * SYS_INVGRIDPW, 8 * SYS_INVGRIDPH)){
        return {-1, -1};
    }

    return
    {
        (locPX - gridPX0) / SYS_INVGRIDPW,
        (locPY - gridPY0) / SYS_INVGRIDPH,
    };
}
