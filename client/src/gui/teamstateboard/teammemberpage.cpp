#include <format>
#include <utility>
#include "totype.hpp"
#include "processrun.hpp"
#include "teamstateboard.hpp"
#include "teammemberpage.hpp"

TeamMemberPage::TeamMemberPage(TeamStateBoard *board, Widget::VarBool argShow)
    : Widget
      {{
          .x = TeamStateBoard::m_pageX,
          .y = TeamStateBoard::m_pageY,
          .w = TeamStateBoard::m_pageW,
          .h = TeamStateBoard::m_pageH,

          .attrs
          {
              .inst
              {
                  .show = std::move(argShow),
                  .moveOnFocus = false,
              },
          },
          .parent{board},
      }}

    , m_board(board)

    , m_title
      {{
          .textFunc = [this]
          {
              return std::format("队伍成员：{}", m_board->m_teamMemberList.memberList.size());
          },
          .parent{this},
      }}

    , m_listView
      {{
          .y = 24,
          .w = TeamStateBoard::m_pageW,
          .rowCount = 12,

          .columnList
          {
              {.title = u8"名字", .w = 150, .text = [this](size_t i){ return m_board->m_teamMemberList.memberList.at(i).name; }},
              {.title = u8"等级", .w =  60, .text = [this](size_t i){ return std::to_string(m_board->m_teamMemberList.memberList.at(i).level); }},
              {.title = u8"职位", .w =  80, .text = [this](size_t i)
              {
                  return std::string(to_cstr((m_board->m_teamMemberList.memberList.at(i).uid == m_board->m_teamMemberList.teamLeader) ? u8"队长" : u8"队员"));
              }},
          },

          .size = [this]{ return m_board->m_teamMemberList.memberList.size(); },
          .selectable = true,

          .parent{this},
      }}

    , m_leave
      {{
          .x = 0,
          .y = TeamStateBoard::m_pageH - 28,

          .textFunc = to_cstr(u8"离开队伍"),
          .onTrigger = [this](Widget *, int)
          {
              m_board->m_processRun->requestLeaveTeam(m_board->m_processRun->getMyHeroUID());
          },

          .attrs
          {
              .show = [this]{ return !m_board->isTeamLeader(); },
          },
          .parent{this},
      }}

    , m_kick
      {{
          .dir = DIR_UPRIGHT,
          .x = TeamStateBoard::m_pageW - 160,
          .y = TeamStateBoard::m_pageH - 28,

          .textFunc = to_cstr(u8"踢出队伍"),
          .onTrigger = [this](Widget *, int)
          {
              if(const auto member = selectedMember(); !member){
                  m_board->m_processRun->addCBLog(CBLOG_ERR, u8"请先选择一名队员");
              }
              else if(member->uid == m_board->m_processRun->getMyHeroUID()){
                  m_board->m_processRun->addCBLog(CBLOG_ERR, u8"不能踢出自己");
              }
              else{
                  m_board->m_processRun->requestLeaveTeam(member->uid);
              }
          },

          .attrs
          {
              .show = [this]{ return m_board->isTeamLeader(); },
          },
          .parent{this},
      }}

    , m_transfer
      {{
          .dir = DIR_UPRIGHT,
          .x = TeamStateBoard::m_pageW - 80,
          .y = TeamStateBoard::m_pageH - 28,

          .textFunc = to_cstr(u8"转让队长"),
          .onTrigger = [this](Widget *, int)
          {
              if(const auto member = selectedMember(); !member){
                  m_board->m_processRun->addCBLog(CBLOG_ERR, u8"请先选择一名队员");
              }
              else if(member->uid == m_board->m_processRun->getMyHeroUID()){
                  m_board->m_processRun->addCBLog(CBLOG_ERR, u8"你已经是队长");
              }
              else{
                  m_board->m_processRun->addCBLog(CBLOG_SYS, u8"转让队长给%s：暂未实现", member->name.c_str());
              }
          },

          .attrs
          {
              .show = [this]{ return m_board->isTeamLeader(); },
          },
          .parent{this},
      }}

    , m_disband
      {{
          .dir = DIR_UPRIGHT,
          .x = TeamStateBoard::m_pageW,
          .y = TeamStateBoard::m_pageH - 28,

          .textFunc = to_cstr(u8"解散队伍"),
          .onTrigger = [this](Widget *, int)
          {
              // server disbands the team when leader leaves
              m_board->m_processRun->requestLeaveTeam(m_board->m_processRun->getMyHeroUID());
          },

          .attrs
          {
              .show = [this]{ return m_board->isTeamLeader(); },
          },
          .parent{this},
      }}
{}

const SDTeamPlayer *TeamMemberPage::selectedMember() const
{
    if(const auto index = m_listView.selectedIndex()){
        return &m_board->m_teamMemberList.memberList.at(index.value());
    }
    return nullptr;
}
