#include "strf.hpp"
#include "totype.hpp"
#include "colorf.hpp"
#include "dbcomid.hpp"
#include "sdldevice.hpp"
#include "acutionitemlist.hpp"
#include "acutionitemrow.hpp"

extern SDLDevice *g_sdlDevice;
AcutionItemRow::AcutionItemRow(AcutionItemList *itemList, size_t rowIndex)
    : Widget
      {{
          .w = m_rowW,
          .h = m_rowH,
      }}

    , m_itemList(fflcheck(itemList))
    , m_rowIndex(fflcheck(rowIndex, rowIndex < AcutionItemList::m_rowCount))

    , m_background
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](const Widget *, int drawDstX, int drawDstY)
          {
              if(const auto itemIndex = m_itemList->itemIndex(m_rowIndex); itemIndex.has_value()){
                  if     (m_itemList->selectedIndex() == itemIndex){ g_sdlDevice->fillRectangle(colorf::RGBA(  0,  80, 255, 96), drawDstX, drawDstY, w(), h()); }
                  else if(m_itemList-> hoveredIndex() == itemIndex){ g_sdlDevice->fillRectangle(colorf::RGBA(255, 255, 255, 48), drawDstX, drawDstY, w(), h()); }
              }
          },
          .parent{this},
      }}

    , m_item
      {{
          .dir = DIR_NONE,
          .x = m_columnCenter_item,
          .y = [this]{ return h() / 2; },
          .textFunc = [this]() -> std::string
          {
              if(const auto entry = m_itemList->item(m_rowIndex)){
                  return to_cstr(DBCOM_ITEMRECORD(entry->item.itemID).name);
              }
              return {};
          },
          .font
          {
              .id = 1,
              .size = 11,
              .color = colorf::WHITE_A255,
          },
          .parent{this},
      }}

    , m_seller
      {{
          .dir = DIR_NONE,
          .x = m_columnCenter_seller,
          .y = [this]{ return h() / 2; },
          .textFunc = [this]() -> std::string
          {
              if(const auto entry = m_itemList->item(m_rowIndex)){
                  return entry->seller;
              }
              return {};
          },
          .font
          {
              .id = 1,
              .size = 11,
              .color = colorf::WHITE_A255,
          },
          .parent{this},
      }}

    , m_timeLeft
      {{
          .dir = DIR_NONE,
          .x = m_columnCenter_time,
          .y = [this]{ return h() / 2; },
          .textFunc = [this]() -> std::string
          {
              if(const auto entry = m_itemList->item(m_rowIndex)){
                  return m_itemList->formatTimeLeft(*entry);
              }
              return {};
          },
          .font
          {
              .id = 1,
              .size = 11,
              .color = colorf::WHITE_A255,
          },
          .parent{this},
      }}

    , m_price
      {{
          .dir = DIR_NONE,
          .x = m_columnCenter_price,
          .y = [this]{ return h() / 2; },
          .textFunc = [this]() -> std::string
          {
              if(const auto entry = m_itemList->item(m_rowIndex)){
                  return str_ksep(entry->price);
              }
              return {};
          },
          .font
          {
              .id = 1,
              .size = 11,
              .color = [this]
              {
                  if(const auto entry = m_itemList->item(m_rowIndex)){
                      return priceColor(entry->price);
                  }
                  return colorf::WHITE_A255;
              },
          },
          .parent{this},
      }}
{}

bool AcutionItemRow::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        if(event.type == SDL_EVENT_MOUSE_MOTION){
            m_itemList->setHoveredRow(m_rowIndex, false);
        }
        return false;
    }

    switch(event.type){
        case SDL_EVENT_MOUSE_MOTION:
            {
                const bool hovered = valid
                    && m_itemList->itemIndex(m_rowIndex).has_value()
                    && m.in(to_d(event.motion.x), to_d(event.motion.y));

                m_itemList->setHoveredRow(m_rowIndex, hovered);
                return false;
            }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                if(!valid){
                    return false;
                }

                if(true
                        && event.button.button == SDL_BUTTON_LEFT
                        && m_itemList->itemIndex(m_rowIndex).has_value()
                        && m.in(to_d(event.button.x), to_d(event.button.y))){
                    m_itemList->selectRow(m_rowIndex);
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        default:
            {
                return Widget::processEventDefault(event, valid, m);
            }
    }
}

uint32_t AcutionItemRow::priceColor(size_t price)
{
    if(price >= 10000000){
        return colorf::RED_A255;
    }
    else if(price >= 1000000){
        return colorf::CYAN_A255;
    }
    else{
        return colorf::YELLOW_A255;
    }
}
