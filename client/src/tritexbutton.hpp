#pragma once
#include <cstdint>
#include <functional>
#include "buttonbase.hpp"

class TritexButton: public ButtonBase
{
    protected:
        using ButtonBase::SeffIDList;
        using ButtonBase::OverCBFunc;
        using ButtonBase::ClickCBFunc;
        using ButtonBase::TriggerCBFunc;

    private:
        struct TritexIDList final
        {
            std::optional<uint32_t> off  = std::nullopt;
            std::optional<uint32_t> on   = std::nullopt;
            std::optional<uint32_t> down = std::nullopt;

            decltype(auto) operator[](this auto && self, size_t index)
            {
                switch(index){
                    case  0: return self.off;
                    case  1: return self.on;
                    case  2: return self.down;
                    default: throw fflerror("index out of range: %zu", index);
                }
            }
        };

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            TritexButton::TritexIDList texIDList {};
            TritexButton::SeffIDList seff {};

            TritexButton::OverCBFunc onOverIn  = nullptr;
            TritexButton::OverCBFunc onOverOut = nullptr;

            TritexButton::ClickCBFunc onClick = nullptr;
            TritexButton::TriggerCBFunc onTrigger = nullptr;

            int offXOnOver = 0;
            int offYOnOver = 0;

            int offXOnClick = 0;
            int offYOnClick = 0;

            bool onClickDone = true;
            bool radioMode   = false;
            bool alterColor  = true;

            Widget::InitAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        TritexButton::TritexIDList m_texIDList;

    private:
        double m_accuBlinkTime = 0.0;
        std::optional<std::tuple<unsigned, unsigned, unsigned>> m_blinkTime = {}; // {off, on} in ms

    private:
        const bool m_alterColor;

    public:
        TritexButton(TritexButton::InitArgs);

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        void setTexID(const TritexButton::TritexIDList &texIDList)
        {
            m_texIDList = texIDList;
        }

        void setBlinkTime(unsigned offTime, unsigned onTime, unsigned activeTotalTime = 0)
        {
            m_blinkTime = std::make_tuple(offTime, onTime, activeTotalTime);
            m_accuBlinkTime = 0.0;
        }

        void stopBlink()
        {
            m_blinkTime.reset();
            m_accuBlinkTime = 0.0;
        }

    public:
        void updateDefault(double fUpdateTime) override
        {
            if(m_blinkTime.has_value()){
                m_accuBlinkTime += fUpdateTime;
                if(const auto activeTotalTime = std::get<2>(m_blinkTime.value()); activeTotalTime > 0 && m_accuBlinkTime > activeTotalTime){
                    m_blinkTime.reset();
                }
            }
        }
};
