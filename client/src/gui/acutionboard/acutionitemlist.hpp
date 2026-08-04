#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include "raiitimer.hpp"
#include "widget.hpp"
#include "itembox.hpp"
#include "serdesmsg.hpp"

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
        std::optional<size_t> m_firstIndex;
        std::optional<size_t> m_selectedRow;
        std::optional<size_t> m_hoveredRow;

    private:
        ItemBox m_itemBox;

    public:
        explicit AcutionItemList(AcutionItemList::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void setItemList(SDAcutionItemList);
        const SDAcutionItemList &getItemList() const
        {
            return m_sdAcutionItemList;
        }

    public:
        std::optional<size_t> firstIndex() const
        {
            return m_firstIndex;
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
                fflassert(m_firstIndex.has_value());
                return m_firstIndex.value() + m_selectedRow.value();
            }
            return {};
        }

        std::optional<size_t> hoveredIndex() const
        {
            if(m_hoveredRow.has_value()){
                fflassert(m_firstIndex.has_value());
                return m_firstIndex.value() + m_hoveredRow.value();
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
            if(m_firstIndex.has_value() && rowIndex < m_rowCount){
                return indexItem(m_firstIndex.value() + rowIndex);
            }
            return nullptr;
        }

        bool canMoveBackward() const
        {
            return m_firstIndex.has_value() && m_firstIndex.value() > 0;
        }

        bool canMoveForward() const
        {
            return m_firstIndex.has_value() && m_firstIndex.value() + m_rowCount < m_sdAcutionItemList.itemList.size();
        }

        void moveBackward();
        void moveForward();

    private:
        void setFirstIndex(size_t);
        void setSelectedRow(size_t, bool);
        void setHoveredRow(size_t, bool);
        void validateRowIndex(size_t) const;

        std::string formatTimeLeft(const SDAcutionItem &);
};
