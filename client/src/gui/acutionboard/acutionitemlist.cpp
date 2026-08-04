#include <algorithm>
#include <utility>
#include "strf.hpp"
#include "totype.hpp"
#include "dbcomid.hpp"
#include "acutionitemrow.hpp"
#include "acutionitemlist.hpp"

AcutionItemList::AcutionItemList(AcutionItemList::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),
          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::nullopt,
          .h = std::nullopt,

          .attrs
          {
              .inst = std::move(args.attrs),
          },
          .parent = std::move(args.parent),
      }}

    , m_itemBox
      {{
          .fixed = AcutionItemRow::m_rowW,

          .headSpace = 1,
          .itemSpace = 1,
          .tailSpace = 1,

          .parent{this},
      }}
{
    for(size_t i = 0; i < m_rowCount; ++i){
        m_itemBox.addItem(new AcutionItemRow(this, i), true);
    }
}

bool AcutionItemList::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        if(m_hoveredRow.has_value()){
            setHoveredRow(m_hoveredRow.value(), false);
        }
        return false;
    }

    if(true
            && valid
            && m_firstIndex.has_value()
            && event.type == SDL_EVENT_MOUSE_WHEEL
            && m.in(to_d(event.wheel.mouse_x), to_d(event.wheel.mouse_y))){
        if(event.wheel.y > 0.0F){
            setFirstIndex(m_firstIndex.value() > 0 ? m_firstIndex.value() - 1 : 0);
        }
        else if(event.wheel.y < 0.0F){
            setFirstIndex(std::min(m_firstIndex.value() + 1, m_sdAcutionItemList.itemList.size() - 1));
        }
        return true;
    }

    return Widget::processEventDefault(event, valid, m);
}

void AcutionItemList::setItemList(SDAcutionItemList sdAIL)
{
    fflassert(sdAIL.category >= ACUTIONCAT_BEGIN, sdAIL.category);
    fflassert(sdAIL.category <  ACUTIONCAT_END  , sdAIL.category);

    for(const auto &entry: sdAIL.itemList){
        fflassert(entry.item);
        fflassert(DBCOM_ITEMRECORD(entry.item.itemID));
    }

    m_sdAcutionItemList = std::move(sdAIL);
    m_firstIndex = m_sdAcutionItemList.itemList.empty() ? std::nullopt : std::optional<size_t>(0);
    m_selectedRow = m_firstIndex.has_value() ? std::optional<size_t>(0) : std::nullopt;
    m_hoveredRow.reset();
    m_acutionTimer.reset();
}

void AcutionItemList::moveBackward()
{
    if(!canMoveBackward()){
        return;
    }

    setFirstIndex(m_firstIndex.value() > m_rowCount ? m_firstIndex.value() - m_rowCount : 0);
    setSelectedRow(0, true);
}

void AcutionItemList::moveForward()
{
    if(!canMoveForward()){
        return;
    }

    setFirstIndex(m_firstIndex.value() + m_rowCount);
    setSelectedRow(0, true);
}

void AcutionItemList::setFirstIndex(size_t firstIndex)
{
    if(!indexItem(firstIndex)){
        throw fflpanic("invalid acution item first index: index = {}, item count = {}", firstIndex, m_sdAcutionItemList.itemList.size());
    }

    m_firstIndex = firstIndex;

    if(m_selectedRow.has_value() && !rowItem(m_selectedRow.value())){
        setSelectedRow(m_selectedRow.value(), false);
    }

    if(m_hoveredRow.has_value() && !rowItem(m_hoveredRow.value())){
        setHoveredRow(m_hoveredRow.value(), false);
    }
}

void AcutionItemList::setSelectedRow(size_t rowIndex, bool selected)
{
    validateRowIndex(rowIndex);

    if(selected){
        if(!rowItem(rowIndex)){
            throw fflpanic("acution item row has no item: row = {}, first index = {}, item count = {}",
                    rowIndex,
                    m_firstIndex.value_or(0),
                    m_sdAcutionItemList.itemList.size());
        }
        m_selectedRow = rowIndex;
    }
    else if(m_selectedRow == rowIndex){
        m_selectedRow.reset();
    }
}

void AcutionItemList::setHoveredRow(size_t rowIndex, bool hovered)
{
    validateRowIndex(rowIndex);

    if(hovered){
        if(!rowItem(rowIndex)){
            throw fflpanic("acution item row has no item: row = {}, first index = {}, item count = {}",
                    rowIndex,
                    m_firstIndex.value_or(0),
                    m_sdAcutionItemList.itemList.size());
        }
        m_hoveredRow = rowIndex;
    }
    else if(m_hoveredRow == rowIndex){
        m_hoveredRow.reset();
    }
}

void AcutionItemList::validateRowIndex(size_t rowIndex) const
{
    if(rowIndex >= m_rowCount){
        throw fflpanic("invalid acution item row: row = {}, row count = {}", rowIndex, m_rowCount);
    }
}

std::string AcutionItemList::formatTimeLeft(const SDAcutionItem &item)
{
    const auto elapsedSecs = m_acutionTimer.diff_sec();
    const auto timeLeft = (item.timeLeft > elapsedSecs) ? (item.timeLeft - elapsedSecs) : 0;

    if(timeLeft == 0){
        return to_cstr(u8"已到期");
    }

    if(timeLeft >= 24 * 60 * 60){
        return to_cstr(str_printf(u8"%zu天%02zu时", timeLeft / (24 * 60 * 60), timeLeft / (60 * 60) % 24));
    }

    if(timeLeft >= 60 * 60){
        return to_cstr(str_printf(u8"%zu时%02zu分", timeLeft / (60 * 60), timeLeft / 60 % 60));
    }

    if(timeLeft >= 60){
        return to_cstr(str_printf(u8"%zu分%02zu秒", timeLeft / 60, timeLeft % 60));
    }

    return to_cstr(str_printf(u8"%zu秒", timeLeft));
}
