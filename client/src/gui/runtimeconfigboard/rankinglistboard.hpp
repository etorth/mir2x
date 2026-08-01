#pragma once
#include "widget.hpp"
#include "itembox.hpp"
#include "scrollcontainer.hpp"
#include "serdesmsg.hpp"

class RankingListBoard final: public Widget
{
    private:
        constexpr static int m_viewportW = 420;
        constexpr static int m_viewportH = 350;

    private:
        const RankingType m_type;

    private:
        ItemBox m_itemBox;
        ScrollContainer m_scroll;

    public:
        explicit RankingListBoard(RankingType);

    public:
        void setRankingList(const SDRankingList &);
};
