#pragma once
#include <tuple>
#include <string>
#include <cstdint>
#include <variant>
#include <SDL2/SDL.h>

#include "colorf.hpp"
#include "widget.hpp"
#include "imageboard.hpp"

class TextBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarStrFunc textFunc {};
            Widget::FontConfig font {};

            Widget::VarBlendMode blendMode = SDL_BLENDMODE_BLEND;

            Widget::InstAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        Widget::FontConfig m_font;
        Widget::VarStrFunc m_textFunc;

    private:
        ImageBoard m_image;

    public:
        TextBoard(TextBoard::InitArgs);

    public:
        void setFont(uint8_t argFont)
        {
            m_font.id = argFont;
        }

        void setFontSize(uint8_t argFontSize)
        {
            m_font.size = argFontSize;
        }

        void setFontStyle(uint8_t argFontStyle)
        {
            m_font.style = argFontStyle;
        }

        void setFontColor(Widget::VarU32 argColor)
        {
            m_font.color = std::move(argColor);
        }

        void setTextFunc(Widget::VarStrFunc argTextFunc)
        {
            m_textFunc = std::move(argTextFunc);
        }

    public:
        std::tuple<std::string, std::string> fontName() const;

    public:
        std::string getText() const
        {
            return Widget::evalStrFunc(m_textFunc, this);
        }

    public:
        bool empty() const
        {
            return getText().empty();
        }

        void drawDefault(Widget::ROIMap m) const override
        {
            return m_image.draw(m);
        }
};
