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

    public:
        using TrigfxFunc = std::variant<std::nullptr_t,
                                        std::function<const Widget *(                   )>,
                                        std::function<const Widget *(                int)>,
                                        std::function<const Widget *(const Widget *, int)>>;

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            TrigfxButton::TrigfxFunc      gfxFunc {};
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

            Widget::InstAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        TrigfxButton::TrigfxFunc      m_gfxFunc;
        std::array<const Widget *, 3> m_gfxList;

    public:
        TrigfxButton(TrigfxButton::InitArgs);

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        void setGfxFunc(TrigfxButton::TrigfxFunc gfxFunc)
        {
            m_gfxFunc = gfxFunc;
        }

        void setGfxList(const std::array<const Widget *, 3> &gfxList)
        {
            m_gfxList = gfxList;
        }

    private:
        const Widget *evalGfxWidget     (std::optional<int> = std::nullopt) const;
        const Widget *evalGfxWidgetValid(                                 ) const; // search the first valid gfx pointer

    public:
        int w() const override { if(auto gfxPtr = evalGfxWidgetValid()){ return gfxPtr->w(); } else { return 0; }}
        int h() const override { if(auto gfxPtr = evalGfxWidgetValid()){ return gfxPtr->h(); } else { return 0; }}
};
