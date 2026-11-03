#pragma once
#include <vector>
#include <string>
#include <cstddef>
#include <optional>
#include <functional>
#include "widget.hpp"
#include "gfxshapeboard.hpp"

class TeamListView: public Widget
{
    private:
        friend class TeamListRow;

    public:
        struct Column final
        {
            const char8_t *title = nullptr;
            int w = 0;

            std::function<std::string(size_t)> text = nullptr;
        };

        struct RowButton final
        {
            const char8_t *label = nullptr;

            std::function<bool(size_t)> show    = nullptr; // nullptr means always show
            std::function<void(size_t)> onClick = nullptr;
        };

        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            int w = 0;
            size_t rowCount = 10;

            std::vector<TeamListView::Column>       columnList {};
            std::vector<TeamListView::RowButton> buttonList {};

            std::function<size_t()> size = nullptr;
            bool selectable = false;

            Widget::InstAttrs attrs {};
            Widget::WADPair  parent {};
        };

    public:
        constexpr static int m_headerH = 22;
        constexpr static int m_rowH    = 22;
        constexpr static int m_barW    =  6;

    private:
        const size_t m_rowCount;

    private:
        const std::vector<TeamListView::Column>       m_columnList;
        const std::vector<TeamListView::RowButton> m_buttonList;

    private:
        const std::function<size_t()> m_size;
        const bool m_selectable;

    private:
        size_t m_firstIndex = 0;

    private:
        std::optional<size_t> m_hoveredRow;
        std::optional<size_t> m_selectedIndex;

    private:
        GfxShapeBoard m_frame;

    public:
        explicit TeamListView(TeamListView::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        size_t size() const
        {
            return m_size ? m_size() : 0;
        }

    public:
        size_t firstIndex() const; // clamped when data shrinks
        std::optional<size_t> rowDataIndex(size_t) const;

    public:
        std::optional<size_t> selectedIndex() const;
        void clearSelection()
        {
            m_selectedIndex.reset();
        }

    private:
        void setHoveredRow(size_t, bool);
};
