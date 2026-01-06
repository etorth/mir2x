#pragma once
#include "gfxtriangle.hpp"
#include "trigfxbutton.hpp"

class GfxDirButton: public TrigfxButton
{
    private:
        struct TriangleArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarSizeOpt w = std::nullopt;
            Widget::VarSizeOpt h = std::nullopt;

            Widget::VarU32 color = colorf::WHITE_A255;
        };

        struct FrameArgs final
        {
            Widget::VarBool show = true;
            Widget::VarU32 color = colorf::WHITE_A255;
        };

        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSize w = 0;
            Widget::VarSize h = 0;

            GfxDirButton::TriangleArgs triangle {};
            GfxDirButton::FrameArgs frame {};

            TrigfxButton::TriggerCBFunc onTrigger = nullptr;
            Widget::WADPair parent {};
        };

    private:
        GfxShapeBoard m_gfxDrawer;

    public:
        GfxDirButton(GfxDirButton::InitArgs);
};
