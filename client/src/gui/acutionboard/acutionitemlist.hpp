#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include "widget.hpp"
#include "itembox.hpp"
#include "serdesmsg.hpp"

class AcutionItemRow;
class AcutionItemList final: public Widget
{
    public:
        constexpr static size_t m_rowCount = 13;
        constexpr static int m_rowW = 460;
        constexpr static int m_rowH = 19;

    public:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::InstAttrs attrs {};
            Widget::WADPair parent {};
        };

    private:
        SDAcutionItemList m_sdAcutionItemList;
        size_t m_firstIndex = 0;
        std::optional<size_t> m_selectedIndex;
        std::optional<size_t> m_hoveredRow;
        double m_elapsedMS = 0.0;

    private:
        ItemBox m_itemBox;

    public:
        explicit AcutionItemList(AcutionItemList::InitArgs);

    public:
        void updateDefault(double) override;
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
            return m_sdAcutionItemList.itemList.empty()
                ? 0
                : m_sdAcutionItemList.itemList.size() - 1;
        }

        void setFirstIndex(size_t);

    public:
        std::optional<size_t> selectedIndex() const;
        std::optional<size_t> hoveredIndex() const;
        void setSelectedIndex(size_t);

    private:
        friend class AcutionItemRow;

        const SDAcutionItem *item(size_t) const;
        std::optional<size_t> itemIndex(size_t) const;

        void selectRow(size_t);
        void setHoveredRow(size_t, bool);

        size_t currentTimeLeft(const SDAcutionItem &) const;
        static std::string formatTimeLeft(size_t);
};
