#include "strf.hpp"
#include "totype.hpp"
#include "colorf.hpp"
#include "dbcomid.hpp"
#include "sdldevice.hpp"
#include "acutionitemlist.hpp"
#include "acutionitemrow.hpp"

extern SDLDevice *g_sdlDevice;

namespace
{
    constexpr int itemColumnW = 108;
    constexpr int sellerColumnW = 140;
    constexpr int timeColumnW = 70;
    constexpr int priceColumnW = 142;

    static_assert(itemColumnW + sellerColumnW + timeColumnW + priceColumnW == AcutionItemList::m_rowW);

    constexpr int itemColumnCenter = itemColumnW / 2;
    constexpr int sellerColumnCenter = itemColumnW + sellerColumnW / 2;
    constexpr int timeColumnCenter = itemColumnW + sellerColumnW + timeColumnW / 2;
    constexpr int priceColumnCenter = itemColumnW + sellerColumnW + timeColumnW + priceColumnW / 2;

    uint32_t priceColor(size_t price)
    {
        if(price >= 10000000){
            return colorf::RED_A255;
        }

        if(price >= 1000000){
            return colorf::CYAN_A255;
        }
        return colorf::YELLOW_A255;
    }
}

AcutionItemRow::AcutionItemRow(AcutionItemList *itemList, size_t rowIndex)
    : Widget
      {{
          .w = AcutionItemList::m_rowW,
          .h = AcutionItemList::m_rowH,
      }}

    , m_itemList(fflcheck(itemList))
    , m_rowIndex(fflcheck(rowIndex, rowIndex < AcutionItemList::m_rowCount))

    , m_background
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](const Widget *, int drawDstX, int drawDstY)
          {
              const auto itemIndex = m_itemList->itemIndex(m_rowIndex);
              if(!itemIndex.has_value()){
                  return;
              }

              if(m_itemList->selectedIndex() == itemIndex){
                  g_sdlDevice->fillRectangle(colorf::RGBA(0, 80, 255, 96), drawDstX, drawDstY, w(), h());
              }
              else if(m_itemList->hoveredIndex() == itemIndex){
                  g_sdlDevice->fillRectangle(colorf::RGBA(255, 255, 255, 48), drawDstX, drawDstY, w(), h());
              }
          },
          .parent{this},
      }}

    , m_itemName
      {{
          .dir = DIR_NONE,
          .x = itemColumnCenter,
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
          .x = sellerColumnCenter,
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
          .x = timeColumnCenter,
          .y = [this]{ return h() / 2; },
          .textFunc = [this]() -> std::string
          {
              if(const auto entry = m_itemList->item(m_rowIndex)){
                  return AcutionItemList::formatTimeLeft(m_itemList->currentTimeLeft(*entry));
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
          .x = priceColumnCenter,
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
