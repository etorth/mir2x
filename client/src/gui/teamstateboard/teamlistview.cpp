#include <algorithm>
#include <utility>
#include "totype.hpp"
#include "colorf.hpp"
#include "sdldevice.hpp"
#include "textboard.hpp"
#include "teamlistrow.hpp"
#include "teamlistview.hpp"

extern SDLDevice *g_sdlDevice;
TeamListView::TeamListView(TeamListView::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),
          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = args.w,
          .h = m_headerH + to_d(args.rowCount) * m_rowH + 2,

          .attrs
          {
              .inst = [&args]
              {
                  auto attrs = std::move(args.attrs);
                  attrs.moveOnFocus = false;
                  return attrs;
              }(),
          },
          .parent = std::move(args.parent),
      }}

    , m_rowCount(args.rowCount)
    , m_columnList(std::move(args.columnList))
    , m_buttonList(std::move(args.buttonList))
    , m_size(std::move(args.size))
    , m_selectable(args.selectable)

    , m_frame
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::A_SHF(96), drawDstX, drawDstY, self->w(), self->h(), 5);
              g_sdlDevice->fillRectangle(colorf::RGBA(231, 231, 189, 40), drawDstX + 1, drawDstY + 1, self->w() - 2, m_headerH - 1, 4);
              g_sdlDevice->drawLine     (colorf::RGBA(231, 231, 189, 80), drawDstX + 1, drawDstY + m_headerH, drawDstX + self->w() - 2, drawDstY + m_headerH);
              g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 100), drawDstX, drawDstY, self->w(), self->h(), 5);

              if(const auto count = size(); count > m_rowCount){
                  const int trackY = m_headerH + 2;
                  const int trackH = self->h() - trackY - 2;
                  const int thumbH = std::max<int>(10, trackH * to_d(m_rowCount) / to_d(count));
                  const int thumbY = trackY + (trackH - thumbH) * to_d(firstIndex()) / to_d(count - m_rowCount);

                  g_sdlDevice->fillRectangle(colorf::RGBA(231, 231, 189, 128), drawDstX + self->w() - m_barW - 1, drawDstY + thumbY, m_barW - 2, thumbH, 2);
              }
          },

          .parent{this},
      }}
{
    int columnX = 1;
    for(const auto &column: m_columnList){
        new TextBoard
        {{
            .dir = DIR_LEFT,
            .x = columnX + 4,
            .y = m_headerH / 2,

            .textFunc = to_cstr(column.title),
            .font
            {
                .color = colorf::RGBA(231, 231, 189, 255),
            },

            .parent{this, true},
        }};
        columnX += column.w;
    }

    for(size_t rowIndex = 0; rowIndex < m_rowCount; ++rowIndex){
        new TeamListRow(this, rowIndex, w() - m_barW - 3);
    }
}

bool TeamListView::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        m_hoveredRow.reset();
        return false;
    }

    if(valid && (event.type == SDL_EVENT_MOUSE_WHEEL) && m.in(to_d(event.wheel.mouse_x), to_d(event.wheel.mouse_y))){
        if(const auto count = size(); count > m_rowCount){
            if(event.wheel.y > 0.0F){
                m_firstIndex = (firstIndex() > 0) ? (firstIndex() - 1) : 0;
            }
            else if(event.wheel.y < 0.0F){
                m_firstIndex = std::min<size_t>(firstIndex() + 1, count - m_rowCount);
            }
            m_hoveredRow.reset();
        }
        return true;
    }

    return Widget::processEventDefault(event, valid, m);
}

size_t TeamListView::firstIndex() const
{
    if(const auto count = size(); count > m_rowCount){
        return std::min<size_t>(m_firstIndex, count - m_rowCount);
    }
    return 0;
}

std::optional<size_t> TeamListView::rowDataIndex(size_t rowIndex) const
{
    if(rowIndex < m_rowCount){
        if(const auto index = firstIndex() + rowIndex; index < size()){
            return index;
        }
    }
    return std::nullopt;
}

std::optional<size_t> TeamListView::selectedIndex() const
{
    if(m_selectedIndex.has_value() && m_selectedIndex.value() < size()){
        return m_selectedIndex;
    }
    return std::nullopt;
}

void TeamListView::setHoveredRow(size_t rowIndex, bool hovered)
{
    if(hovered){
        m_hoveredRow = rowIndex;
    }
    else if(m_hoveredRow == rowIndex){
        m_hoveredRow.reset();
    }
}
