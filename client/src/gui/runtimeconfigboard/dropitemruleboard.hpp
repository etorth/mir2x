#pragma once
#include <cstdint>
#include "widget.hpp"
#include "itembox.hpp"
#include "scrollcontainer.hpp"

class RuntimeConfigBoard;
class DropItemRuleBoard final: public Widget
{
    private:
        constexpr static int m_viewportW = 420;
        constexpr static int m_viewportH = 300;

    private:
        constexpr static int m_sliderW = 20;

    private:
        RuntimeConfigBoard *m_configBoard = nullptr;

    private:
        ItemBox m_itemBox;

    private:
        ScrollContainer m_scroll;

    public:
        explicit DropItemRuleBoard(RuntimeConfigBoard *);
};
