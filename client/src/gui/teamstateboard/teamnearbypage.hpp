#pragma once
#include <vector>
#include <string>
#include "widget.hpp"
#include "textbutton.hpp"
#include "checklabel.hpp"
#include "teamlistview.hpp"
#include "serdesmsg.hpp"

class TeamStateBoard;
class TeamNearbyPage: public Widget
{
    private:
        TeamStateBoard *m_board;

    private:
        std::vector<SDTeamInfo> m_teamList;

    private:
        bool m_visible = false; // query nearby teams when page turns visible

    private:
        bool m_onlyMatch = false;
        std::vector<size_t> m_viewList; // index into m_teamList

    private:
        CheckLabel m_onlyMatchCheck;

    private:
        TeamListView m_listView;

    private:
        TextButton m_refresh;
        TextButton m_pickPlayer;

    public:
        TeamNearbyPage(TeamStateBoard *, Widget::VarBool);

    public:
        void updateDefault(double) override;

    public:
        void queryTeamList();
        void setTeamInfo(SDTeamInfo);

    private:
        const SDTeamInfo &viewTeam(size_t index) const
        {
            return m_teamList.at(m_viewList.at(index));
        }

    private:
        void updateViewList();
};
