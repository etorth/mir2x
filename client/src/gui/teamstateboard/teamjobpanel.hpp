#pragma once
#include <cstddef>
#include <optional>
#include <functional>
#include "widget.hpp"
#include "textboard.hpp"
#include "textinput.hpp"
#include "textbutton.hpp"
#include "checklabel.hpp"
#include "scrollcontainer.hpp"
#include "serdesmsg.hpp"

class TeamJobPanel: public Widget // full page to setup requirement of one job
{
    private:
        constexpr static int m_titleH = 28;
        constexpr static int m_inputRowH = 28;

    private:
        constexpr static int m_magicColumnCount = 3;
        constexpr static int m_magicRowH = 24;
        constexpr static int m_magicLabelY = m_titleH + m_inputRowH + 14;
        constexpr static int m_magicViewY = m_magicLabelY + 14;

    public:
        struct InitArgs final
        {
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            int w = 0;
            int h = 0;

            size_t jobIndex = 0;
            std::optional<SDTeamJobReq> *requirement = nullptr;

            std::function<void()> onBack = nullptr;

            Widget::VarBool show = true;
            Widget::VarBool editable = true;

            Widget::WADPair parent {};
        };

    private:
        const size_t m_jobIndex;
        std::optional<SDTeamJobReq> * const m_requirement;

    private:
        TextBoard m_title;

    private:
        TextInput m_minLevel;
        TextInput m_maxLevel;
        TextInput m_minStat;

    private:
        TextBoard       m_magicLabel;
        Widget          m_magicCanvas;
        ScrollContainer m_magicScroll;

    private:
        TextButton m_reset;
        TextButton m_back;

    public:
        explicit TeamJobPanel(TeamJobPanel::InitArgs);

    public:
        void loadInput();
        void resetInput();
        void applyInput();

    public:
        static void setupNumberInput(TextInput &);
        static int  parseNumberInput(const TextInput &, int = 0); // input, value if empty

    private:
        void buildMagicList();
};
