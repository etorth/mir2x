#include <algorithm>
#include <cmath>
#include <utility>
#include "colorf.hpp"
#include "dbcomid.hpp"
#include "invpack.hpp"
#include "labelboard.hpp"
#include "mathf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "stallitemgrid.hpp"

extern PNGTexDB *g_itemDB;
extern SDLDevice *g_sdlDevice;

StallItemGrid::StallItemGrid(StallItemGrid::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),
          .x = std::move(args.x),
          .y = std::move(args.y),
          .w = 249,
          .h = m_gridH * SYS_INVGRIDPH,
          .parent = std::move(args.parent),
      }}

    , m_onSelect(std::move(args.onSelect))

    , m_slider
      {{
          .bar
          {
              .x = 241,
              .y = 6,
              .w = 8,
              .h = 212,
              .v = true,
          },
          .index = 0,
          .parent{this},
      }}
{
    m_slider.setActive([this]{ return maxStartRow() > 0; });
}

void StallItemGrid::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        m_hoveredIndex = -1;
        return;
    }

    const int startOffX = m.x - m.ro->x;
    const int startOffY = m.y - m.ro->y;
    const auto [mousePX, mousePY] = g_sdlDevice->getMousePLoc();

    m_hoveredIndex = getPackBinIndex(mousePX - startOffX, mousePY - startOffY);

    const size_t firstRow = startRow();
    const int selected = selectedIndex();

    for(int i = 0; i < std::ssize(m_packBinList); ++i){
        const auto fillColor = [i, selected, this]() -> uint32_t
        {
            if(i == m_hoveredIndex){
                return colorf::WHITE + colorf::A_SHF(48);
            }
            else if(i == selected){
                return colorf::BLUE + colorf::A_SHF(48);
            }
            else{
                return 0;
            }
        }();
        drawItem(m.x, m.y, firstRow, m_packBinList.at(i), fillColor);
    }

    Widget::drawDefault(m);
}

bool StallItemGrid::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(Widget::processEventDefault(event, valid, m)){
        return true;
    }

    if(!valid){
        return consumeFocus(false);
    }

    const int startOffX = m.x - m.ro->x;
    const int startOffY = m.y - m.ro->y;

    switch(event.type){
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                if(event.button.button != SDL_BUTTON_LEFT){
                    return false;
                }

                const int mouseDX = to_d(event.button.x) - startOffX;
                const int mouseDY = to_d(event.button.y) - startOffY;
                const auto [gridX, gridY] = getGrid(mouseDX, mouseDY);

                if(gridX < 0 || gridY < 0){
                    return false;
                }

                if(const auto index = getPackBinIndex(mouseDX, mouseDY); index >= 0){
                    m_selectedItemID = m_packBinList.at(index).item.itemID;
                    m_selectedSeqID  = m_packBinList.at(index).item.seqID;
                }
                else{
                    clearSelection();
                }

                if(m_onSelect){
                    m_onSelect();
                }
                return consumeFocus(true);
            }
        case SDL_EVENT_MOUSE_WHEEL:
            {
                const int mouseDX = to_d(event.wheel.mouse_x) - startOffX;
                const int mouseDY = to_d(event.wheel.mouse_y) - startOffY;
                const auto [gridX, gridY] = getGrid(mouseDX, mouseDY);

                if(gridX >= 0 && gridY >= 0){
                    if(const auto maxRow = maxStartRow(); maxRow > 0){
                        const int direction = (to_d(event.wheel.y) > 0) ? -1 : 1;
                        setStartRow(to_uz(std::clamp<int>(to_d(startRow()) + direction, 0, to_d(maxRow))));
                    }
                    return consumeFocus(true);
                }
                return false;
            }
        default:
            {
                return false;
            }
    }
}

void StallItemGrid::setItemList(const std::vector<SDStallItem> &itemList)
{
    const size_t oldStartRow = startRow();

    m_packBinList.clear();
    m_priceList.clear();
    m_hoveredIndex = -1;

    Pack2D pack2D(m_gridW);
    for(const auto &stallItem: itemList){
        auto bin = InvPack::makePackBin(stallItem.item);
        if(bin.w > m_gridW){
            continue;
        }

        pack2D.add(bin);
        m_packBinList.push_back(std::move(bin));
        m_priceList.push_back(stallItem.price);
    }

    if(selectedIndex() < 0){
        clearSelection();
    }
    setStartRow(oldStartRow);
}

const SDItem *StallItemGrid::selectedItem() const
{
    if(const auto index = selectedIndex(); index >= 0){
        return std::addressof(m_packBinList.at(index).item);
    }
    return nullptr;
}

size_t StallItemGrid::selectedPrice() const
{
    if(const auto index = selectedIndex(); index >= 0){
        return m_priceList.at(index);
    }
    return 0;
}

void StallItemGrid::clearSelection()
{
    m_selectedItemID = 0;
    m_selectedSeqID  = 0;
}

const SDItem *StallItemGrid::hoveredItem() const
{
    if(m_hoveredIndex >= 0 && m_hoveredIndex < std::ssize(m_packBinList)){
        return std::addressof(m_packBinList.at(m_hoveredIndex).item);
    }
    return nullptr;
}

size_t StallItemGrid::hoveredPrice() const
{
    if(m_hoveredIndex >= 0 && m_hoveredIndex < std::ssize(m_priceList)){
        return m_priceList.at(m_hoveredIndex);
    }
    return 0;
}

int StallItemGrid::selectedIndex() const
{
    if(m_selectedItemID == 0){
        return -1;
    }

    for(int i = 0; i < std::ssize(m_packBinList); ++i){
        if(m_packBinList.at(i).item.itemID == m_selectedItemID && m_packBinList.at(i).item.seqID == m_selectedSeqID){
            return i;
        }
    }
    return -1;
}

size_t StallItemGrid::rowCount() const
{
    size_t result = 0;
    for(const auto &bin: m_packBinList){
        result = std::max<size_t>(result, to_uz(bin.y + bin.h));
    }
    return result;
}

size_t StallItemGrid::maxStartRow() const
{
    const size_t rows = rowCount();
    return (rows > m_gridH) ? rows - m_gridH : 0;
}

size_t StallItemGrid::startRow() const
{
    return to_uz(std::lround(maxStartRow() * m_slider.getValue()));
}

void StallItemGrid::setStartRow(size_t row)
{
    const size_t maxRow = maxStartRow();
    const size_t newRow = std::min<size_t>(row, maxRow);
    m_slider.setValue(maxRow ? static_cast<float>(newRow) / static_cast<float>(maxRow) : 0.0F, false);
}

std::tuple<int, int> StallItemGrid::getGrid(int locPDX, int locPDY) const
{
    if(!mathf::pointInRectangle<int>(locPDX, locPDY, 0, 0, m_gridW * SYS_INVGRIDPW, m_gridH * SYS_INVGRIDPH)){
        return {-1, -1};
    }

    return
    {
        locPDX / SYS_INVGRIDPW,
        locPDY / SYS_INVGRIDPH + to_d(startRow()),
    };
}

int StallItemGrid::getPackBinIndex(int locPDX, int locPDY) const
{
    const auto [gridX, gridY] = getGrid(locPDX, locPDY);
    if(gridX < 0 || gridY < 0){
        return -1;
    }

    for(int i = 0; i < std::ssize(m_packBinList); ++i){
        const auto &bin = m_packBinList.at(i);
        if(mathf::pointInRectangle<int>(gridX, gridY, bin.x, bin.y, bin.w, bin.h)){
            return i;
        }
    }
    return -1;
}

void StallItemGrid::drawItem(int dstX, int dstY, size_t firstRow, const PackBin &bin, uint32_t fillColor) const
{
    if(!(bin && bin.x >= 0 && bin.y >= 0 && bin.w > 0 && bin.h > 0)){
        return;
    }

    auto texPtr = g_itemDB->retrieve(DBCOM_ITEMRECORD(bin.item.itemID).pkgGfxID | 0X01000000);
    if(!texPtr){
        return;
    }

    const int startX = dstX;
    const int startY = dstY - to_d(firstRow) * SYS_INVGRIDPH;

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
                dstX, dstY,
                m_gridW * SYS_INVGRIDPW,
                m_gridH * SYS_INVGRIDPH)){
        g_sdlDevice->drawTexture(texPtr, drawDstX, drawDstY, drawSrcX, drawSrcY, drawSrcW, drawSrcH);
    }

    int binGridX = bin.x;
    int binGridY = bin.y;
    int binGridW = bin.w;
    int binGridH = bin.h;

    if(mathf::rectangleOverlapRegion<int>(
                0, to_d(firstRow),
                m_gridW, m_gridH,
                binGridX, binGridY,
                binGridW, binGridH)){
        g_sdlDevice->fillRectangle(
                fillColor,
                startX + binGridX * SYS_INVGRIDPW,
                startY + binGridY * SYS_INVGRIDPH,
                binGridW * SYS_INVGRIDPW,
                binGridH * SYS_INVGRIDPH);

        if(bin.item.count > 1){
            const LabelBoard itemCount
            {{
                .label = to_u8rawstr(std::to_string(bin.item.count)).c_str(),
                .font
                {
                    .id = 1,
                    .size = 10,
                    .color = colorf::YELLOW_A255,
                },
            }};

            itemCount.draw(
            {
                .dir = DIR_UPRIGHT,
                .x = startX + (binGridX + binGridW) * SYS_INVGRIDPW,
                .y = startY + binGridY * SYS_INVGRIDPH - 2,
            });
        }
    }
}
