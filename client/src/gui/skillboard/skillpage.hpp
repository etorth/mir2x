#pragma once
#include <cstdint>
#include <vector>
#include <span>
#include "processrun.hpp"
#include "widget.hpp"
#include "imageboard.hpp"

class MagicIconButton;
class SkillBoardConfig;
class SkillPage: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            uint32_t pageTexID = SYS_U32NIL;

            SkillBoardConfig *config = nullptr;
            ProcessRun       *proc   = nullptr;

            Widget::WADPair parent {};
        };

    private:
        SkillBoardConfig * const m_config;
        ProcessRun       * const m_processRun;

    private:
        std::vector<MagicIconButton *> m_magicIconButtonList;

    private:
        ImageBoard m_bg;

    public:
        SkillPage(SkillPage::InitArgs);

    public:
        void addIcon(uint32_t);

    public:
        const auto & getMagicIconButtonList() const
        {
            return m_magicIconButtonList;
        }
};
