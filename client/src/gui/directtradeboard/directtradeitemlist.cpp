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
#include "directtradeitemlist.hpp"

extern PNGTexDB *g_itemDB;
extern SDLDevice *g_sdlDevice;

DirectTradeItemList::DirectTradeItemList(DirectTradeItemList::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),
          .x = std::move(args.x),
          .y = std::move(args.y),
          .w = 213,
          .h = 207,
          .parent = std::move(args.parent),
      }}

    , m_onClick(std::move(args.onClick))

    , m_slider
      {{
          .bar
          {
              .x = 200,
              .y = 7,
              .w = 8,
              .h = 193,
              .v = true,
          },
          .index = 0,
          .parent{this},
      }}
{
    m_slider.setActive([this]{ return maxStartRow() > 0; });
}

void DirectTradeItemList::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        m_hoveredIndex = -1;
        return;
    }

    const int startOffX = m.x - m.ro->x;
    const int startOffY = m.y - m.ro->y;
    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();

    m_hoveredIndex = getPackBinIndex(mousePX - startOffX, mousePY - startOffY);
    const size_t firstRow = startRow();
    for(int i = 0; i < std::ssize(m_packBinList); ++i){
        drawItem(m.x, m.y, firstRow, m_packBinList.at(i),
                (i == m_hoveredIndex) ? colorf::WHITE + colorf::A_SHF(48) : 0);
    }

    Widget::drawDefault(m);
}

bool DirectTradeItemList::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
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
                if(gridX >= 0 && gridY >= 0){
                    if(m_onClick){
                        m_onClick(ClickEvent
                        {
                            .packBinIndex = getPackBinIndex(mouseDX, mouseDY),
                            .gridX = gridX,
                            .gridY = gridY,
                        });
                    }
                    return consumeFocus(true);
                }

                return m.in(to_d(event.button.x), to_d(event.button.y)) ? consumeFocus(true) : false;
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

std::vector<SDItem> DirectTradeItemList::itemList() const
{
    std::vector<SDItem> result;
    result.reserve(m_packBinList.size());
    for(const auto &bin: m_packBinList){
        result.push_back(bin.item);
    }
    return result;
}

void DirectTradeItemList::setItemList(std::vector<SDItem> itemList)
{
    const auto sameItem = [](const SDItem &lhs, const SDItem &rhs)
    {
        return lhs.itemID == rhs.itemID
            && lhs.seqID == rhs.seqID
            && lhs.count == rhs.count
            && lhs.duration[0] == rhs.duration[0]
            && lhs.duration[1] == rhs.duration[1]
            && lhs.extAttrList == rhs.extAttrList;
    };

    if(itemList.size() == m_packBinList.size()){
        bool sameItemList = true;
        for(size_t i = 0; i < itemList.size(); ++i){
            if(!sameItem(itemList.at(i), m_packBinList.at(i).item)){
                sameItemList = false;
                break;
            }
        }

        if(sameItemList){
            return;
        }
    }

    const size_t oldStartRow = startRow();
    clear();
    for(auto &item: itemList){
        fflassert(addItem(std::move(item)));
    }
    setStartRow(oldStartRow);
}

bool DirectTradeItemList::addItem(SDItem item, int x, int y)
{
    fflassert(item);

    auto bin = InvPack::makePackBin(std::move(item));
    if(bin.w > m_gridW){
        return false;
    }

    Pack2D pack2D(m_gridW);
    for(const auto &currBin: m_packBinList){
        fflassert(pack2D.put(currBin.x, currBin.y, currBin.w, currBin.h));
    }

    if(x >= 0 && y >= 0 && x + bin.w <= m_gridW && !pack2D.occupied(x, y, bin.w, bin.h, true)){
        bin.x = x;
        bin.y = y;
        pack2D.occupy(bin.x, bin.y, bin.w, bin.h, true);
    }
    else{
        pack2D.add(bin);
    }

    const size_t oldStartRow = startRow();
    const size_t itemEndRow = to_uz(bin.y + bin.h);
    m_packBinList.push_back(std::move(bin));

    setStartRow((itemEndRow > oldStartRow + m_gridH) ? itemEndRow - m_gridH : oldStartRow);
    return true;
}

PackBin DirectTradeItemList::takeItem(size_t index)
{
    fflassert(index < m_packBinList.size(), index, m_packBinList.size());

    const size_t oldStartRow = startRow();
    auto bin = std::move(m_packBinList.at(index));
    m_packBinList.erase(m_packBinList.begin() + static_cast<std::ptrdiff_t>(index));
    setStartRow(std::min<size_t>(oldStartRow, maxStartRow()));
    return bin;
}

std::vector<SDItem> DirectTradeItemList::takeItemList()
{
    std::vector<SDItem> result;
    result.reserve(m_packBinList.size());
    for(auto &bin: m_packBinList){
        result.push_back(std::move(bin.item));
    }
    clear();
    return result;
}

void DirectTradeItemList::clear()
{
    m_packBinList.clear();
    m_hoveredIndex = -1;
    m_slider.setValue(0.0F, false);
}

const SDItem *DirectTradeItemList::hoveredItem() const
{
    if(m_hoveredIndex >= 0 && m_hoveredIndex < std::ssize(m_packBinList)){
        return std::addressof(m_packBinList.at(m_hoveredIndex).item);
    }
    return nullptr;
}

size_t DirectTradeItemList::rowCount() const
{
    size_t result = 0;
    for(const auto &bin: m_packBinList){
        result = std::max<size_t>(result, to_uz(bin.y + bin.h));
    }
    return result;
}

size_t DirectTradeItemList::maxStartRow() const
{
    const size_t rows = rowCount();
    return (rows > m_gridH) ? rows - m_gridH : 0;
}

size_t DirectTradeItemList::startRow() const
{
    return to_uz(std::lround(maxStartRow() * m_slider.getValue()));
}

void DirectTradeItemList::setStartRow(size_t row)
{
    const size_t maxRow = maxStartRow();
    const size_t newRow = std::min<size_t>(row, maxRow);
    m_slider.setValue(maxRow ? static_cast<float>(newRow) / static_cast<float>(maxRow) : 0.0F, false);
}

std::tuple<int, int> DirectTradeItemList::getGrid(int locPDX, int locPDY) const
{
    if(!mathf::pointInRectangle<int>(
                locPDX,
                locPDY,
                m_gridX,
                m_gridY,
                m_gridW * SYS_INVGRIDPW,
                m_gridH * SYS_INVGRIDPH)){
        return {-1, -1};
    }

    return
    {
        (locPDX - m_gridX) / SYS_INVGRIDPW,
        (locPDY - m_gridY) / SYS_INVGRIDPH + to_d(startRow()),
    };
}

int DirectTradeItemList::getPackBinIndex(int locPDX, int locPDY) const
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

void DirectTradeItemList::drawItem(int dstX, int dstY, size_t firstRow, const PackBin &bin, uint32_t fillColor) const
{
    if(!(bin && bin.x >= 0 && bin.y >= 0 && bin.w > 0 && bin.h > 0)){
        return;
    }

    auto texPtr = g_itemDB->retrieve(DBCOM_ITEMRECORD(bin.item.itemID).pkgGfxID | 0X01000000);
    if(!texPtr){
        return;
    }

    const int startX = dstX + m_gridX;
    const int startY = dstY + m_gridY - to_d(firstRow) * SYS_INVGRIDPH;
    const int viewX = dstX + m_gridX;
    const int viewY = dstY + m_gridY;

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
                viewX, viewY,
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
