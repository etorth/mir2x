#pragma once
#include <deque>
#include "mathf.hpp"
#include "widget.hpp"
#include "sysconst.hpp"
#include "labelboard.hpp"
#include "protocoldef.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class TeamStateBoard: public Widget
{
    // following is the original texture
    // to create full board, middle part needs to repeat
    //
    // +----------------------------------+
    // |                                  |
    // | x---o----------------------+---+ |
    // | |   |                      |   | |
    // | |   |                      |   | |<------+
    // | |   |                      |   | |       |
    // | |   |                      |   | |  texRepeatH: as texture resource to repeat
    // | |   |                      |   | |       |
    // | |   |                      |   | |<------+
    // | |   |                      |   | |
    // | +---+----------------------o---x |             +
    // |                                  |             |
    // +----------------------------------+             +-+: original texture rectangle
    //       ^                      ^
    //       |                      |                   X
    //       +----uidTextRegionW----+                   |
    //                                                  +-X: uidRegion
    //
    //                                                  o
    //                                                  |
    //                                                  +-o: uidTextRegion

    private:
        const int m_uidRegionX = 13;
        const int m_uidRegionY = 80;
        const int m_uidRegionW = 231;
        const int m_uidRegionH = 98;
        const int m_texRepeatH = 70;

        const int m_lineSpace = 4;        // space between two XMLTypeset
        const int m_uidTextRegionW = 220; // XMLTypeset max width, margin excluded

        const int m_uidMinCount = 5;
        const int m_uidMaxCount = 10;

    private:
        const uint8_t  m_font      = 1;
        const uint8_t  m_fontSize  = 12;
        const uint8_t  m_fontStyle = 0;

        const uint32_t m_fontColor       = colorf::WHITE + colorf::A_SHF(255);
        const uint32_t m_hoveredColor    = colorf::BLUE  + colorf::A_SHF(100);
        const uint32_t m_selectedBGColor = colorf::RED   + colorf::A_SHF(100);

    private:
        bool m_showCandidateList = false;

    private:
        int m_startIndex[2] {0, 0};
        int m_selectedIndex[2] {-1, -1};

    private:
        std::deque<std::pair<SDTeamCandidate, hres_timer>> m_teamCandidateList;

    private:
        SDTeamMemberList m_teamMemberList;

    private:
        TritexButton m_enableTeam;

    private:
        TritexButton m_switchShow;
        TritexButton m_addMember;
        TritexButton m_deleteMember;
        TritexButton m_refresh;

    private:
        TritexButton m_close;

    private:
        ProcessRun *m_processRun;

    public:
        TeamStateBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    public:
        int lineHeight() const
        {
            return XMLTypeset(-1, LALIGN_LEFT, false, m_font, m_fontSize, m_fontStyle).getDefaultFontHeight() + m_lineSpace;
        }

        size_t lineShowCount() const
        {
            return mathf::bound<size_t>(lineCount(), m_uidMinCount, m_uidMaxCount);
        }

        size_t lineCount() const
        {
            return m_showCandidateList ? m_teamCandidateList.size() : m_teamMemberList.memberList.size();
        }

    public:
        void refresh();
        void addTeamCandidate(SDTeamCandidate);
        void setTeamMemberList(SDTeamMemberList);

    private:
        void adjustButtonPos();

    public:
        const auto &getTeamMemberList() const
        {
            return m_teamMemberList;
        }

    private:
        const SDTeamPlayer &getSDTeamPlayer(int index) const
        {
            if(m_showCandidateList){
                return m_teamCandidateList.at(index).first.player;
            }
            else{
                return m_teamMemberList.memberList.at(index);
            }
        }
};
