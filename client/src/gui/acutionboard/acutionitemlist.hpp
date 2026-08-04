#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include "raiitimer.hpp"
#include "serdesmsg.hpp"
#include "widget.hpp"
#include "itembox.hpp"

class AcutionItemList final: public Widget
{
    private:
        friend class AcutionBoard;
        friend class AcutionItemRow;

    private:
        constexpr static size_t m_rowCount = 13;

    public:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::InstAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        hres_timer m_acutionTimer;
        SDAcutionItemList m_sdAcutionItemList;

    private:
        size_t m_firstIndex = 0; // valid only when empty() is false

    private:
        std::optional<size_t> m_hoveredRow;
        std::optional<size_t> m_selectedRow;

    private:
        ItemBox m_itemBox;

    public:
        explicit AcutionItemList(AcutionItemList::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void setItemList(SDAcutionItemList);

    public:
        const SDAcutionItemList &getItemList() const
        {
            return m_sdAcutionItemList;
        }

    public:
        bool empty() const
        {
            return m_sdAcutionItemList.itemList.empty();
        }

        size_t size() const
        {
            return m_sdAcutionItemList.itemList.size();
        }

    public:
        std::optional<size_t> firstIndex() const
        {
            return empty() ? std::nullopt : std::optional<size_t>(m_firstIndex);
        }

        std::optional<size_t> selectedRow() const
        {
            return m_selectedRow;
        }

        std::optional<size_t> hoveredRow() const
        {
            return m_hoveredRow;
        }

        std::optional<size_t> selectedIndex() const
        {
            if(m_selectedRow.has_value()){
                fflassert(!empty());
                return m_firstIndex + m_selectedRow.value();
            }
            return {};
        }

        std::optional<size_t> hoveredIndex() const
        {
            if(m_hoveredRow.has_value()){
                fflassert(!empty());
                return m_firstIndex + m_hoveredRow.value();
            }
            return {};
        }

        const SDAcutionItem *indexItem(size_t itemIndex) const
        {
            if(itemIndex < m_sdAcutionItemList.itemList.size()){
                return &m_sdAcutionItemList.itemList.at(itemIndex);
            }
            return nullptr;
        }

        const SDAcutionItem *rowItem(size_t rowIndex) const
        {
            if(!empty() && rowIndex < m_rowCount){
                return indexItem(m_firstIndex + rowIndex);
            }
            return nullptr;
        }

    public:
        bool canMovePrev() const
        {
            return !empty() && m_firstIndex > 0;
        }

        bool canMoveNext() const
        {
            return !empty() && (m_firstIndex + m_rowCount) < m_sdAcutionItemList.itemList.size();
        }

        void movePrev();
        void moveNext();

    private:
        void setFirstIndex(size_t);

    private:
        void setSelectedRow(size_t, bool);
        void  setHoveredRow(size_t, bool);

    private:
        std::string formatTimeLeft(const SDAcutionItem &);
};
