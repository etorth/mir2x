#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include "buttonbase.hpp"

class TrigfxButton: public ButtonBase
{
    protected:
        using ButtonBase::   OverCBFunc;
        using ButtonBase::  ClickCBFunc;
        using ButtonBase::TriggerCBFunc;
        using ButtonBase::SeffIDList;

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            std::array<const Widget *, 3> gfxList {};
            TrigfxButton::SeffIDList seff {};

            TrigfxButton::OverCBFunc onOverIn  = nullptr;
            TrigfxButton::OverCBFunc onOverOut = nullptr;

            TrigfxButton::ClickCBFunc onClick = nullptr;
            TrigfxButton::TriggerCBFunc onTrigger = nullptr;

            int offXOnOver = 0;
            int offYOnOver = 0;

            int offXOnClick = 0;
            int offYOnClick = 0;

            bool onClickDone = true;
            bool radioMode   = false;

            Widget::InitAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        std::array<const Widget *, 3> m_gfxList;

    public:
        TrigfxButton(TrigfxButton::InitArgs);

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        void setGfxList(const std::array<const Widget *, 3> &gfxList)
        {
            m_gfxList = gfxList;
        }
};
