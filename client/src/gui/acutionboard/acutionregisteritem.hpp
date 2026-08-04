#pragma once
#include <functional>
#include <utility>
#include "widget.hpp"
#include "imageboard.hpp"
#include "textboard.hpp"
#include "serdesmsg.hpp"

class AcutionRegisterItem final: public Widget
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
        SDItem m_item;

    private:
        const std::function<void()> m_onClick;

    private:
        ImageBoard m_image;
        TextBoard m_name;

    public:
        explicit AcutionRegisterItem(AcutionRegisterItem::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void setInputEnabled(bool enabled)
        {
            m_enabled = enabled;
        }

    public:
        const SDItem &item() const
        {
            return m_item;
        }

        void setItem(SDItem item)
        {
            m_item = std::move(item);
        }

        SDItem takeItem()
        {
            return std::exchange(m_item, SDItem{});
        }

        void clear()
        {
            m_item = {};
        }

    private:
        SDL_Texture *itemTexture() const;
        std::pair<int, int> itemImageSize() const;
};
