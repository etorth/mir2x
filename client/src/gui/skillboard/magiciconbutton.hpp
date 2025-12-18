#pragma once
#include <cstdint>
#include "widget.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class SkillBoardConfig;
class MagicIconButton: public Widget
{
    // +-+-----+
    // |A|     |
    // +-+     |
    // |       |
    // +-------+-+
    //         |1|
    //         +-+

    private:
        struct InitArgs final
        {
            uint32_t magicID = 0;

            SkillBoardConfig *config = nullptr;
            ProcessRun       *proc   = nullptr;

            Widget::WADPair parent {};
        };

    private:
        const uint32_t m_magicID;

    private:
        SkillBoardConfig * const m_config;
        ProcessRun       * const m_processRun;

    private:
        TritexButton m_icon;

    public:
        MagicIconButton(MagicIconButton::InitArgs);

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        bool cursorOn() const
        {
            return m_icon.getState() != BEVENT_OFF;
        }

    public:
        uint32_t magicID() const
        {
            return m_magicID;
        }
};
