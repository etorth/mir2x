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
        size_t m_firstIndex = 0;
        std::optional<size_t> m_hoveredRow;
        std::optional<size_t> m_selectedIndex;

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
        size_t firstIndex() const
        {
            return m_firstIndex;
        }

        size_t maxFirstIndex() const
        {
            return m_sdAcutionItemList.itemList.empty() ? 0 : (m_sdAcutionItemList.itemList.size() - 1);
        }

        void setFirstIndex(size_t firstIndex)
        {
            m_firstIndex = std::min<size_t>(firstIndex, maxFirstIndex());
        }

        void setSelectedIndex(size_t itemIndex)
        {
            if(itemIndex < m_sdAcutionItemList.itemList.size()){
                m_selectedIndex = itemIndex;
            }
            else{
                m_selectedIndex.reset();
            }
        }

    public:
        std::optional<size_t> selectedIndex() const
        {
            if(m_selectedIndex.has_value() && m_selectedIndex.value() < m_sdAcutionItemList.itemList.size()){
                return m_selectedIndex;
            }
            return {};
        }

        std::optional<size_t> hoveredIndex() const
        {
            if(m_hoveredRow.has_value()){
                return itemIndex(m_hoveredRow.value());
            }
            return {};
        }

    public:
        const SDAcutionItem *item(size_t) const;
        std::optional<size_t> itemIndex(size_t) const;

        void selectRow(size_t);
        void setHoveredRow(size_t, bool);

    private:
        std::string formatTimeLeft(const SDAcutionItem &);
};
