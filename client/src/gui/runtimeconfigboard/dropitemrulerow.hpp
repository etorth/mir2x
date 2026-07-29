#pragma once
#include <cstdint>
#include "widget.hpp"
#include "checklabel.hpp"
#include "labelboard.hpp"
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
        LabelBoard m_name;
        CheckLabel m_highlight;
        CheckLabel m_filter;

    public:
        DropItemRuleRow(RuntimeConfigBoard *, uint32_t);
};
