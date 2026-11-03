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
        struct NearbyTeam final
        {
            std::string leader {};

            int job   = 0;
            int level = 0;

            int memberCount = 0;
            SDTeamRequirement requirement {};
        };

    private:
        TeamStateBoard *m_board;

    private:
        std::vector<NearbyTeam> m_teamList;

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

    private:
        const NearbyTeam &viewTeam(size_t index) const
        {
            return m_teamList.at(m_viewList.at(index));
        }

    private:
        void updateViewList();
};
