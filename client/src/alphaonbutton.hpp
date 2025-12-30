#pragma once
#include "imageboard.hpp"
#include "trigfxbutton.hpp"

class AlphaOnButton: public TrigfxButton
{
    protected:
        using TrigfxButton::   OverCBFunc;
        using TrigfxButton::  ClickCBFunc;
        using TrigfxButton::TriggerCBFunc;

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            int onOffX = 0;
            int onOffY = 0;

            int onRadius = 0;

            Widget::VarU32 modColor  = colorf::WHITE_A255;
            Widget::VarU32 downTexID = 0U;

            AlphaOnButton::OverCBFunc onOverIn  = nullptr;
            AlphaOnButton::OverCBFunc onOverOut = nullptr;

            AlphaOnButton::ClickCBFunc onClick = nullptr;
            AlphaOnButton::TriggerCBFunc onTrigger = nullptr;

            bool triggerOnDone = true;

            Widget::WADPair parent {};
        };

    private:
        const int m_onOffX;
        const int m_onOffY;
        const int m_onRadius;

    private:
        ImageBoard m_down;
        Widget     m_on;  // hold cover
        Widget     m_off; // placeholder for off state, no gfx effect

    public:
        AlphaOnButton(AlphaOnButton::InitArgs);
};
