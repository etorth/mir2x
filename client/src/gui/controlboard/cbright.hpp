#pragma once
#include <cstdint>
#include <functional>
#include "widget.hpp"
#include "acbutton.hpp"
#include "textboard.hpp"
#include "imageboard.hpp"
#include "tritexbutton.hpp"
#include "gfxcropboard.hpp"
#include "alphaonbutton.hpp"

class ProcessRun;
class ControlBoard;
class CBRight: public Widget
{
    private:
        friend class ControlBoard;

    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard   m_bgFull;
        GfxCropBoard m_bg;

    private:
        AlphaOnButton m_buttonExchange;
        AlphaOnButton m_buttonMiniMap;
        AlphaOnButton m_buttonMagicKey;

    private:
        TritexButton m_buttonInventory;
        TritexButton m_buttonHeroState;
        TritexButton m_buttonHeroMagic;

    private:
        TritexButton m_buttonGuild;
        TritexButton m_buttonTeam;
        TritexButton m_buttonQuest;
        TritexButton m_buttonHorse;
        TritexButton m_buttonRuntimeConfig;
        TritexButton m_buttonFriendChat;

    private:
        ACButton m_buttonAC;
        ACButton m_buttonDC;

    public:
        CBRight(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                ProcessRun *,
                Widget * = nullptr,
                bool     = false);
};
