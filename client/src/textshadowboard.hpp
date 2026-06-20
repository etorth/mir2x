#pragma once
#include <functional>
#include "widget.hpp"
#include "textboard.hpp"

class TextShadowBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarInt shadowX = 0;
            Widget::VarInt shadowY = 0;

            Widget::VarStrFunc textFunc {};
            Widget::FontConfig font {};

            Widget::VarU32       shadowColor = colorf::BLACK + colorf::A_SHF(128);
            Widget::VarBlendMode blendMode   = SDL_BLENDMODE_BLEND;

            Widget::WADPair parent {};
        };

    private:
        TextBoard m_textShadow;
        TextBoard m_text;

    public:
        TextShadowBoard(TextShadowBoard::InitArgs);
};
