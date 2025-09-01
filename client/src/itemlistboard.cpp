#include "strf.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "purchaseboard.hpp"
#include "inputstringboard.hpp"

extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ItemListBoard::ItemListBoard(int argX, int argY, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          DIR_UPLEFT,
          argX,
          argY,
          m_gfxSrcW,
          m_gfxSrcH,
          {},

          widgetPtr,
          autoDelete
      }

    , m_leftButton
      {
          DIR_UPLEFT,
          25,
          163,
          {SYS_U32NIL, 0X08000007, 0X08000008},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              m_page = to_uz(mathf::bound<int>(to_d(m_page) - 1, 0, to_d(pageCount()) - 1));
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_selectButton
      {
          DIR_UPLEFT,
          67,
          163,
          {SYS_U32NIL, 0X08000005, 0X08000006},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              onSelect();
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_rightButton
      {
          DIR_UPLEFT,
          115,
          163,
          {SYS_U32NIL, 0X08000009, 0X0800000A},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              m_page = to_uz(mathf::bound<int>(to_d(m_page) + 1, 0, to_d(pageCount()) - 1));
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_closeButton
      {
          DIR_UPLEFT,
          158,
          159,
          {SYS_U32NIL, 0X0000001C, 0X0000001D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              onClose();
              setShow(false);
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }
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

bool ItemListBoard::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    const auto roiOpt = cropDrawROI(startDstX, startDstY, roi);
    if(!roiOpt.has_value()){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_leftButton  .processParentEvent(event, valid, startDstX, startDstY, roi)){ return true; }
    if(m_selectButton.processParentEvent(event, valid, startDstX, startDstY, roi)){ return true; }
    if(m_rightButton .processParentEvent(event, valid, startDstX, startDstY, roi)){ return true; }
    if(m_closeButton .processParentEvent(event, valid, startDstX, startDstY, roi)){ return true; }

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
    {
        DIR_UPLEFT,
        0,
        0,
        200,

        to_cstr(getGridHoverLayout(index)),
        0,

        {},
        false,
        false,
        false,
        false,

        1,
        12,
        0,
        colorf::WHITE + colorf::A_SHF(255),
        0,

        LALIGN_JUSTIFY,
    };


    const int margin = 20;
    const int maxWidth = 200;
    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();

    g_sdlDevice->fillRectangle(colorf::RGBA(0, 0, 0, 200), mousePX, mousePY, std::max<int>(hoverTextBoard.w(), maxWidth) + margin * 2, hoverTextBoard.h() + margin * 2);
    hoverTextBoard.drawAt(DIR_UPLEFT, mousePX + margin, mousePY + margin);
}

void ItemListBoard::drawEx(int dstX, int dstY, const Widget::ROIOpt &roi) const
{
    const auto roiOpt = cropDrawROI(dstX, dstY, roi);
    if(!roiOpt.has_value()){
        return;
    }

    if(auto texPtr = g_progUseDB->retrieve(0X08000001)){
        g_sdlDevice->drawTexture(texPtr, dstX, dstY, m_gfxSrcX, m_gfxSrcY, m_gfxSrcW, m_gfxSrcH);
    }

    drawChildEx(&m_leftButton  , dstX, dstY, roiOpt.value());
    drawChildEx(&m_selectButton, dstX, dstY, roiOpt.value());
    drawChildEx(&m_rightButton , dstX, dstY, roiOpt.value());
    drawChildEx(&m_closeButton , dstX, dstY, roiOpt.value());

    const auto fnDrawTitle = [this](const std::u8string &title)
    {
        LabelBoard
        {
            DIR_UPLEFT,
            0,
            0,
            title.c_str(),

            1,
            12,
            0,

            colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
        }.drawAt(DIR_NONE, 99, 18); // (DIR_NONE, x() + 99, y() + 18);
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

            const int remapXDiff = startDstX - roiOpt->x;
            const int remapYDiff = startDstY - roiOpt->y;

            if(auto texPtr = g_itemDB->retrieve(ir.pkgGfxID | 0X02000000)){
                const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
                const int rightDrawX = rightBoxX + (m_boxW - texW) / 2;
                const int rightDrawY = rightBoxY + (m_boxH - texH) / 2;

                g_sdlDevice->drawTexture(texPtr, remapXDiff + rightDrawX, remapYDiff + rightDrawY);
            }

            if(const auto header = getGridHeader(i); !header.empty()){
                LabelBoard
                {
                    DIR_UPLEFT,
                    0,
                    0,
                    to_u8cstr(header),

                    1,
                    10,
                    0,

                    colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
                }.drawAt(DIR_UPLEFT, remapXDiff + rightBoxX, remapYDiff + rightBoxY);
            }

            const bool gridSelected = m_selectedPageGrid.has_value() && (m_selectedPageGrid.value() == i);
            const bool cursorOn = [rightBoxX, rightBoxY, this]() -> bool
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
