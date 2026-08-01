#pragma once
#include <cstddef>
#include <cstdint>
#include "widget.hpp"
#include "itempair.hpp"
#include "gfxshapeboard.hpp"
#include "serdesmsg.hpp"

class RankingRow final: public Widget
{
    private:
        const uint32_t m_dbid;

    private:
        GfxShapeBoard m_bg;
        ItemPair m_pair;

    public:
        RankingRow(Widget::VarSize, size_t, RankingType, const SDRankingEntry &);
};
