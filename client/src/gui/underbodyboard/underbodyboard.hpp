#pragma once
#include "widget.hpp"
#include "textboard.hpp"
#include "itemflex.hpp"

class Hero;
class UnderBodyBoard: public Widget
{
    // lines under a player, name first, then guild, spouse and titles
    // every line reads the hero when drawn and hides itself if it has nothing to show

    private:
        struct InitArgs
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x   = 0;
            Widget::VarInt y   = 0;

            Hero *hero = nullptr;
            Widget::WADPair parent {};
        };

    private:
        Hero *m_hero;

    private:
        TextBoard m_name;
        TextBoard m_guild;
        TextBoard m_spouse;
        ItemFlex  m_titleList;

    private:
        ItemFlex m_flex;

    public:
        UnderBodyBoard(UnderBodyBoard::InitArgs);
};
