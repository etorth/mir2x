#pragma once
#include <deque>
#include <utility>
#include <cstdint>
#include "widget.hpp"
#include "raiitimer.hpp"
#include "serdesmsg.hpp"
#include "gfxshapeboard.hpp"
#include "baseframeboard.hpp"
#include "teammenutab.hpp"
#include "teamnearbypage.hpp"
#include "teammemberpage.hpp"
#include "teamapplicantpage.hpp"
#include "teamrequirementpage.hpp"

class ProcessRun;
class TeamStateBoard: public Widget
{
    private:
        friend class TeamNearbyPage;
        friend class TeamMemberPage;
        friend class TeamApplicantPage;
        friend class TeamRequirementPage;

    private:
        enum PageType: int
        {
            PAGE_NEARBY,
            PAGE_CREATE,
            PAGE_MEMBER,
            PAGE_SETTING,
            PAGE_APPLICANT,
        };

    private:
        constexpr static int m_boardW = 600;
        constexpr static int m_boardH = 440;

    private:
        constexpr static int m_menuX = 30;
        constexpr static int m_menuY = 30;
        constexpr static int m_menuW = 90;

    private:
        constexpr static int m_tabH   = 26;
        constexpr static int m_tabGap =  8;

    private:
        constexpr static int m_pageX = m_menuX + m_menuW + 20;
        constexpr static int m_pageY = 40;
        constexpr static int m_pageW = m_boardW - m_pageX - 50;
        constexpr static int m_pageH = 360;

    private:
        ProcessRun *m_processRun;

    private:
        SDTeamMemberList m_teamMemberList;
        std::deque<std::pair<SDTeamCandidate, hres_timer>> m_teamCandidateList;

    private:

    private:
        PageType m_page = PAGE_NEARBY;

    private:
        BaseFrameBoard m_frameBoard;
        GfxShapeBoard  m_leftMenuBackground;

    private:
        TeamMenuTab m_tabNearby;
        TeamMenuTab m_tabCreate;
        TeamMenuTab m_tabMember;
        TeamMenuTab m_tabSetting;
        TeamMenuTab m_tabApplicant;

    private:
        TeamNearbyPage      m_pageNearby;
        TeamRequirementPage m_pageCreate;
        TeamMemberPage      m_pageMember;
        TeamRequirementPage m_pageSetting;
        TeamApplicantPage   m_pageApplicant;

    public:
        TeamStateBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void addTeamCandidate(SDTeamCandidate);
        void setTeamMemberList(SDTeamMemberList);

    public:
        const auto &getTeamMemberList() const
        {
            return m_teamMemberList;
        }

    private:
        bool inTeam() const;
        bool isTeamLeader() const;

    private:
        PageType currentPage() const;
        bool pageVisible(PageType) const;
};
