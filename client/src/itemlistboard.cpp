#include "strf.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "inputstringboard.hpp"

extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ItemListBoard::ItemListBoard(int argX, int argY, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .x = argX,
          .y = argY,
          .w = m_gfxSrcW,
          .h = m_gfxSrcH,
          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_leftButton
      {{
          .x = 25,
          .y = 163,

          .texIDList
          {
              .on   = 0X08000007,
              .down = 0X08000008,
          },

          .onTrigger = [this](Widget *, int)
          {
              m_page = to_uz(mathf::bound<int>(to_d(m_page) - 1, 0, to_d(pageCount()) - 1));
          },

          .parent{this},
      }}

    , m_selectButton
      {{
          .x = 67,
          .y = 163,

          .texIDList
          {
              .on   = 0X08000005,
              .down = 0X08000006,
          },

          .onTrigger = [this](Widget *, int)
          {
              onSelect();
          },

          .parent{this},
      }}

    , m_rightButton
      {{
          .x = 115,
          .y = 163,

          .texIDList
          {
              .on   = 0X08000009,
              .down = 0X0800000A,
          },

          .onTrigger = [this](Widget *, int)
          {
              m_page = to_uz(mathf::bound<int>(to_d(m_page) + 1, 0, to_d(pageCount()) - 1));
          },

          .parent{this},
      }}

    , m_closeButton
      {{
          .x = 158,
          .y = 159,

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](Widget *, int)
          {
              onClose();
              setShow(false);
          },

          .parent{this},
      }}
{
    setShow(false);
    if(auto texPtr = g_progUseDB->retrieve(0X08000000)){
        setW(SDLDeviceHelper::getTextureWidth (texPtr));
        setH(SDLDeviceHelper::getTextureHeight(texPtr));
    }
    else{
        throw fflerror("no valid purchase status board frame texture");
    }
}

bool ItemListBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_leftButton  .processEventParent(event, valid, m)){ return true; }
    if(m_selectButton.processEventParent(event, valid, m)){ return true; }
    if(m_rightButton .processEventParent(event, valid, m)){ return true; }
    if(m_closeButton .processEventParent(event, valid, m)){ return true; }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(const auto gridIndex = getPageGrid(); gridIndex.has_value() && pageCount() > 0 && m_page * 3 * 4 + gridIndex.value() < itemCount()){
                    m_selectedPageGrid = m_page * 3 * 4 + gridIndex.value();
                }
                break;
            }
        case SDL_MOUSEWHEEL:
            {
                if(getPageGrid().has_value()){
                    if(event.wheel.y > 0){
                        m_page = to_uz(mathf::bound<int>(to_d(m_page) - 1, 0, to_d(pageCount()) - 1));
                    }
                    else{
                        m_page = to_uz(mathf::bound<int>(to_d(m_page) + 1, 0, to_d(pageCount()) - 1));
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

std::optional<size_t> ItemListBoard::getPageGrid() const
{
    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();

    const int onBoardPX = mousePX;// - x();
    const int onBoardPY = mousePY;// - y();

    if(mathf::pointInRectangle<int>(onBoardPX, onBoardPY, m_startX, m_startY, m_boxW * 4, m_boxH * 3)){
        const size_t r = to_uz(onBoardPY - m_startY) / m_boxH;
        const size_t c = to_uz(onBoardPX - m_startX) / m_boxW;
        return r * 4 + c;
    }
    return {};
}

void ItemListBoard::drawGridHoverLayout(size_t index) const
{
    fflassert(pageCount() > 0);
    fflassert(index < itemCount());

    const LayoutBoard hoverTextBoard
    {{
        .lineWidth = 200,
        .initXML = to_cstr(getGridHoverLayout(index)),

        .font
        {
            .id = 1,
            .size = 12,
        },

        .lineAlign = LALIGN_JUSTIFY,
    }};


    const int margin = 20;
    const int maxWidth = 200;
    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();

    g_sdlDevice->fillRectangle(colorf::RGBA(0, 0, 0, 200), mousePX, mousePY, std::max<int>(hoverTextBoard.w(), maxWidth) + margin * 2, hoverTextBoard.h() + margin * 2);
    hoverTextBoard.draw({.x=mousePX + margin, .y=mousePY + margin});
}

void ItemListBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    if(auto texPtr = g_progUseDB->retrieve(0X08000001)){
        g_sdlDevice->drawTexture(texPtr, m.x, m.y, m_gfxSrcX, m_gfxSrcY, m_gfxSrcW, m_gfxSrcH);
    }

    drawChild(&m_leftButton  , m);
    drawChild(&m_selectButton, m);
    drawChild(&m_rightButton , m);
    drawChild(&m_closeButton , m);

    const auto fnDrawTitle = [this](const std::u8string &title)
    {
        LabelBoard
        {{
            .label = title.c_str(),
            .font
            {
                .id = 1,
                .size = 12,
                .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
            },

        }}.draw({.dir=DIR_NONE, .x=99, .y=18}); // (DIR_NONE, x() + 99, y() + 18);
    };

    if(pageCount() > 0){
        fflassert(m_page < pageCount());
        fnDrawTitle(str_printf(u8"第%zu/%zu页", m_page + 1, pageCount()));
    }
    else{
        fnDrawTitle(u8"（空）");
        return;
    }

    int cursorOnGridIndex = -1;
    for(int r = 0; r < 3; ++r){
        for(int c = 0; c < 4; ++c){
            const size_t i = m_page * 3 * 4 + r * 4 + c;
            if(i >= itemCount()){
                break;
            }

            const auto &item = getItem(i);
            const auto &ir = DBCOM_ITEMRECORD(item.itemID);
            fflassert(ir);

            constexpr int rightStartX = m_startX;
            constexpr int rightStartY = m_startY;
            const int rightBoxX = rightStartX + c * m_boxW;
            const int rightBoxY = rightStartY + r * m_boxH;

            const int remapXDiff = m.x - m.ro->x;
            const int remapYDiff = m.y - m.ro->y;

            if(auto texPtr = g_itemDB->retrieve(ir.pkgGfxID | 0X02000000)){
                const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
                const int rightDrawX = rightBoxX + (m_boxW - texW) / 2;
                const int rightDrawY = rightBoxY + (m_boxH - texH) / 2;

                g_sdlDevice->drawTexture(texPtr, remapXDiff + rightDrawX, remapYDiff + rightDrawY);
            }

            if(const auto header = getGridHeader(i); !header.empty()){
                LabelBoard
                {{
                    .label = to_u8cstr(header),
                    .font
                    {
                        .id = 1,
                        .size = 10,
                        .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
                    },
                }}.draw({.x=remapXDiff + rightBoxX, .y=remapYDiff + rightBoxY});
            }

            const bool gridSelected = m_selectedPageGrid.has_value() && (m_selectedPageGrid.value() == i);
            const bool cursorOn = [rightBoxX, rightBoxY, remapXDiff, remapYDiff, this]() -> bool
            {
                const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
                return mathf::pointInRectangle<int>(mousePX, mousePY, remapXDiff + rightBoxX, remapYDiff + rightBoxY, m_boxW, m_boxH);
            }();

            if(gridSelected || cursorOn){
                const uint32_t gridColor = gridSelected ? (colorf::BLUE + colorf::A_SHF(96)) : (colorf::WHITE + colorf::A_SHF(96));
                g_sdlDevice->fillRectangle(gridColor, remapXDiff + rightBoxX, remapYDiff + rightBoxY, m_boxW, m_boxH);
            }

            if(cursorOn){
                cursorOnGridIndex = to_d(i);
            }
        }
    }

    if(cursorOnGridIndex >= 0){
        drawGridHoverLayout(cursorOnGridIndex);
    }
}
