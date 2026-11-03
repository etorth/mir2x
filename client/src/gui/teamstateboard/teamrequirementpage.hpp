#pragma once
#include <array>
#include <cstddef>
#include <optional>
#include <functional>
#include "widget.hpp"
#include "textinput.hpp"
#include "textbutton.hpp"
#include "checklabel.hpp"
#include "teamjobrow.hpp"
#include "teamjobpanel.hpp"
#include "serdesmsg.hpp"

class TeamStateBoard;
class TeamRequirementPage: public Widget
{
    private:
        constexpr static int m_rowH = 28;
        constexpr static int m_jobRowY = m_rowH + 8;
        constexpr static int m_autoAcceptY = m_jobRowY + m_rowH * 3 + 8;
        constexpr static int m_confirmY = m_autoAcceptY + m_rowH + 12;

    private:
        TeamStateBoard *m_board;

    private:
        Widget::VarBool m_editable;

    private:
        std::function<void(const SDTeamRequirement &)> m_onConfirm;

    private:
        SDTeamRequirement m_requirement;

    private:
        std::optional<size_t> m_jobIndex; // per-job setup page, main page if empty

    private:
        Widget m_mainPage;

    private:
        TextInput  m_maxMember;
        CheckLabel m_autoAccept;

    private:
        TextButton m_confirm;

    private:
        std::array<TeamJobPanel *, SDTeamRequirement::jobInfoList.size()> m_jobPanelList {};

    public:
        TeamRequirementPage(TeamStateBoard *, Widget::VarBool, Widget::VarBool, const char8_t *, std::function<void(const SDTeamRequirement &)>);

    public:
        void loadRequirement(const SDTeamRequirement &);

    public:
        bool editable() const
        {
            return Widget::evalBool(m_editable, this);
        }

    public:
        void showMainPage()
        {
            m_jobIndex.reset();
        }

    private:
        void applyInput();
};
