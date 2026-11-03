#pragma once
#include <string>
#include <cstdint>
#include "widget.hpp"
#include "textboard.hpp"
#include "textbutton.hpp"
#include "teamlistview.hpp"

class TeamStateBoard;
class TeamApplicantPage: public Widget
{
    private:
        TeamStateBoard *m_board;

    private:
        TextBoard m_title;
        TeamListView m_listView;

    private:
        TextButton m_clear;

    public:
        TeamApplicantPage(TeamStateBoard *, Widget::VarBool);

    private:
        static std::string waitTimeString(uint64_t);
};
