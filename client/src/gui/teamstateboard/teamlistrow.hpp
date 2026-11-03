#pragma once
#include <cstddef>
#include <optional>
#include "widget.hpp"
#include "gfxshapeboard.hpp"

class TeamListView;
class TeamListRow: public Widget
{
    private:
        TeamListView *m_list;

    private:
        const size_t m_rowIndex;

    private:
        GfxShapeBoard m_background;

    public:
        TeamListRow(TeamListView *, size_t, int);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    private:
        std::optional<size_t> dataIndex() const;
};
