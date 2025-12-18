#pragma once
#include <optional>
#include <functional>
#include "widget.hpp"

class GfxShapeBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSize w = 0;
            Widget::VarSize h = 0;

            Widget::VarDrawFunc drawFunc = nullptr;

            Widget::InstAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        Widget::VarDrawFunc m_drawFunc;

    public:
        explicit GfxShapeBoard(GfxShapeBoard::InitArgs);

    public:
        void drawDefault(Widget::ROIMap) const override;
};
