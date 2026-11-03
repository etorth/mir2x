#include "totype.hpp"
#include "colorf.hpp"
#include "sdldevice.hpp"
#include "textboard.hpp"
#include "textbutton.hpp"
#include "teamlistrow.hpp"
#include "teamlistview.hpp"

extern SDLDevice *g_sdlDevice;
TeamListRow::TeamListRow(TeamListView *list, size_t rowIndex, int rowW)
    : Widget
      {{
          .x = 1,
          .y = TeamListView::m_headerH + 1 + to_d(rowIndex) * TeamListView::m_rowH,

          .w = rowW,
          .h = TeamListView::m_rowH,

          .parent{list, true},
      }}

    , m_list(fflcheck(list))
    , m_rowIndex(rowIndex)

    , m_background
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](const Widget *self, int drawDstX, int drawDstY)
          {
              if(const auto index = dataIndex()){
                  if     (m_list->selectedIndex() == index    ){ g_sdlDevice->fillRectangle(colorf::RGBA(  0,  80, 255, 96), drawDstX, drawDstY, self->w(), self->h()); }
                  else if(m_list->m_hoveredRow    == m_rowIndex){ g_sdlDevice->fillRectangle(colorf::RGBA(255, 255, 255, 48), drawDstX, drawDstY, self->w(), self->h()); }
                  else if(index.value() % 2                     ){ g_sdlDevice->fillRectangle(colorf::RGBA(255, 255, 255, 16), drawDstX, drawDstY, self->w(), self->h()); }
              }
          },

          .parent{this},
      }}
{
    int columnX = 0;
    for(const auto &column: m_list->m_columnList){
        new TextBoard
        {{
            .dir = DIR_LEFT,
            .x = columnX + 4,
            .y = h() / 2,

            .textFunc = [this, &column]() -> std::string
            {
                if(const auto index = dataIndex(); index.has_value() && column.text){
                    return column.text(index.value());
                }
                return {};
            },

            .parent{this, true},
        }};
        columnX += column.w;
    }

    int buttonX = w() - 4;
    for(auto p = m_list->m_buttonList.rbegin(); p != m_list->m_buttonList.rend(); ++p){
        const auto &button = *p;
        const auto buttonPtr = new TextButton
        {{
            .dir = DIR_RIGHT,
            .x = buttonX,
            .y = h() / 2,

            .textFunc = to_cstr(button.label),
            .onTrigger = [this, &button](Widget *, int)
            {
                if(const auto index = dataIndex(); index.has_value() && button.onClick){
                    button.onClick(index.value());
                }
            },

            .attrs
            {
                .show = [this, &button]
                {
                    const auto index = dataIndex();
                    return index.has_value() && (!button.show || button.show(index.value()));
                },
            },

            .parent{this, true},
        }};
        buttonX -= buttonPtr->w() + 8;
    }
}

bool TeamListRow::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this) || !dataIndex().has_value()){
        m_list->setHoveredRow(m_rowIndex, false);
        return false;
    }

    if(event.type == SDL_EVENT_MOUSE_MOTION){
        m_list->setHoveredRow(m_rowIndex, valid && m.in(to_d(event.motion.x), to_d(event.motion.y)));
    }

    if(Widget::processEventDefault(event, valid, m)){
        return true;
    }

    if(valid && m_list->m_selectable && (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) && (event.button.button == SDL_BUTTON_LEFT) && m.in(to_d(event.button.x), to_d(event.button.y))){
        m_list->m_selectedIndex = dataIndex();
        return consumeFocus(true);
    }
    return false;
}

std::optional<size_t> TeamListRow::dataIndex() const
{
    return m_list->rowDataIndex(m_rowIndex);
}
