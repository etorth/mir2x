#pragma once
#include <cstdint>
#include "widget.hpp"
#include "itembox.hpp"
#include "gfxshapeboard.hpp"
#include "scrollcontainer.hpp"

class RuntimeConfigBoard;
class DropItemRuleBoard final: public Widget
{
    private:
        constexpr static int m_listX = 0;
        constexpr static int m_listY = 0;
        constexpr static int m_listW = 420;
        constexpr static int m_listH = 300;

    private:
        RuntimeConfigBoard *m_configBoard = nullptr;

    private:
        GfxShapeBoard m_listBg;

    private:
        ItemBox m_itemBox;
        ScrollContainer m_scroll;

    public:
        explicit DropItemRuleBoard(RuntimeConfigBoard *);
};
