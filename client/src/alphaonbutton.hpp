#pragma once
#include <cstdint>
#include <functional>
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "buttonbase.hpp"
#include "imageboard.hpp"

class AlphaOnButton: public ButtonBase
{
    protected:
        using ButtonBase::   OverCBFunc;
        using ButtonBase::  ClickCBFunc;
        using ButtonBase::TriggerCBFunc;

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
        Widget::VarU32 m_modColor;
        Widget::VarU32 m_downTexID;

    private:
        const int m_onOffX;
        const int m_onOffY;
        const int m_onRadius;

    private:
        ImageBoard m_on;
        ImageBoard m_down;

    public:
        AlphaOnButton(AlphaOnButton::InitArgs);
};
