#pragma once
#include <cstdint>
#include <functional>
#include "widget.hpp"
#include "textboard.hpp"

class AcutionRegisterPrice final: public Widget
{
    public:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            std::function<void()> onClick {};

            Widget::InstAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        bool m_enabled = true;
        uint64_t m_price = 0;

    private:
        const std::function<void()> m_onClick;

    private:
        TextBoard m_text;

    public:
        explicit AcutionRegisterPrice(AcutionRegisterPrice::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void setInputEnabled(bool enabled)
        {
            m_enabled = enabled;
        }

    public:
        uint64_t price() const
        {
            return m_price;
        }

        void setPrice(uint64_t price)
        {
            m_price = price;
        }

        void clear()
        {
            m_price = 0;
        }
};
