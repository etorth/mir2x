#pragma once
#include <cstdint>
#include "widget.hpp"
#include "itempair.hpp"
#include "gfxshapeboard.hpp"

class RuntimeConfigBoard;
class DropItemRuleRow final: public Widget
{
    private:
        RuntimeConfigBoard *m_configBoard = nullptr;

    private:
        const uint32_t m_itemID = 0;

    private:
        GfxShapeBoard m_bg;

    private:
        ItemPair m_pair;

    public:
        DropItemRuleRow(Widget::VarSize, RuntimeConfigBoard *, uint32_t);
};
