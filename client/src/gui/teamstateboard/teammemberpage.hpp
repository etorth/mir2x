#pragma once
#include "widget.hpp"
#include "textboard.hpp"
#include "textbutton.hpp"
#include "serdesmsg.hpp"
#include "teamlistview.hpp"

class TeamStateBoard;
class TeamMemberPage: public Widget
{
    private:
        TeamStateBoard *m_board;

    private:
        TextBoard m_title;
        TeamListView m_listView;

    private:
        TextButton m_leave;

    private:
        TextButton m_kick;
        TextButton m_transfer;
        TextButton m_disband;

    public:
        TeamMemberPage(TeamStateBoard *, Widget::VarBool);

    private:
        const SDTeamPlayer *selectedMember() const;
};
