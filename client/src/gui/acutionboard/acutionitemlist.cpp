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
          .fixed = m_rowW,
          .parent{this},
      }}
{
    for(size_t rowIndex = 0; rowIndex < m_rowCount; ++rowIndex){
        m_itemBox.addItem(new AcutionItemRow(this, rowIndex), true);
    }
}

void AcutionItemList::updateDefault(double updateTime)
{
    m_elapsedMS += updateTime;
    Widget::updateDefault(updateTime);
}

bool AcutionItemList::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        m_hoveredRow.reset();
        return false;
    }

    if(true
            && valid
            && event.type == SDL_EVENT_MOUSE_WHEEL
            && m.in(to_d(event.wheel.mouse_x), to_d(event.wheel.mouse_y))){
        if(event.wheel.y > 0.0F){
            setFirstIndex(m_firstIndex > 0 ? m_firstIndex - 1 : 0);
        }
        else if(event.wheel.y < 0.0F){
            setFirstIndex(m_firstIndex + 1);
        }
        return true;
    }

    return Widget::processEventDefault(event, valid, m);
}

void AcutionItemList::setItemList(SDAcutionItemList sdAcutionItemList)
{
    fflassert(sdAcutionItemList.category >= ACUTIONCAT_BEGIN && sdAcutionItemList.category < ACUTIONCAT_END, sdAcutionItemList.category);
    for(const auto &entry: sdAcutionItemList.itemList){
        fflassert(entry.item);
        fflassert(DBCOM_ITEMRECORD(entry.item.itemID));
    }

    m_sdAcutionItemList = std::move(sdAcutionItemList);
    m_firstIndex = 0;
    m_selectedIndex = m_sdAcutionItemList.itemList.empty() ? std::nullopt : std::optional<size_t>(0);
    m_hoveredRow.reset();
    m_elapsedMS = 0.0;
}

void AcutionItemList::setFirstIndex(size_t firstIndex)
{
    m_firstIndex = std::min(firstIndex, maxFirstIndex());
}

std::optional<size_t> AcutionItemList::selectedIndex() const
{
    if(m_selectedIndex.has_value() && m_selectedIndex.value() < m_sdAcutionItemList.itemList.size()){
        return m_selectedIndex;
    }
    return {};
}

std::optional<size_t> AcutionItemList::hoveredIndex() const
{
    if(m_hoveredRow.has_value()){
        return itemIndex(m_hoveredRow.value());
    }
    return {};
}

void AcutionItemList::setSelectedIndex(size_t itemIndex)
{
    if(itemIndex < m_sdAcutionItemList.itemList.size()){
        m_selectedIndex = itemIndex;
    }
    else{
        m_selectedIndex.reset();
    }
}

const SDAcutionItem *AcutionItemList::item(size_t rowIndex) const
{
    if(const auto index = itemIndex(rowIndex)){
        return &m_sdAcutionItemList.itemList.at(index.value());
    }
    return nullptr;
}

std::optional<size_t> AcutionItemList::itemIndex(size_t rowIndex) const
{
    if(rowIndex < m_rowCount){
        if(const size_t itemIndex = m_firstIndex + rowIndex; itemIndex < m_sdAcutionItemList.itemList.size()){
            return itemIndex;
        }
    }
    return {};
}

void AcutionItemList::selectRow(size_t rowIndex)
{
    if(const auto index = itemIndex(rowIndex)){
        m_selectedIndex = index;
    }
}

void AcutionItemList::setHoveredRow(size_t rowIndex, bool hovered)
{
    if(hovered){
        if(itemIndex(rowIndex).has_value()){
            m_hoveredRow = rowIndex;
        }
    }
    else if(m_hoveredRow == rowIndex){
        m_hoveredRow.reset();
    }
}

size_t AcutionItemList::currentTimeLeft(const SDAcutionItem &entry) const
{
    const size_t elapsedSeconds = to_uz(std::max(0.0, m_elapsedMS) / 1000.0);
    return entry.timeLeft > elapsedSeconds ? entry.timeLeft - elapsedSeconds : 0;
}

std::string AcutionItemList::formatTimeLeft(size_t timeLeft)
{
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
